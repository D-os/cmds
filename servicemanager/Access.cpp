/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Access.h"

#include <android-base/logging.h>
#include <binder/IPCThreadState.h>
#include <log/log_safetynet.h>
#include <selinux/android.h>
#include <selinux/avc.h>

namespace android {

#ifdef VENDORSERVICEMANAGER
constexpr bool kIsVendor = true;
#else
constexpr bool kIsVendor = false;
#endif

static std::string getPidcon(pid_t pid) {
    android_errorWriteLog(0x534e4554, "121035042");

    char* lookup = nullptr;
    if (getpidcon(pid, &lookup) < 0) {
        LOG(ERROR) << "SELinux: getpidcon(pid=" << pid << ") failed to retrieve pid context";
        return "";
    }
    std::string result = lookup;
    freecon(lookup);
    return result;
}

static struct selabel_handle* getSehandle() {
    static struct selabel_handle* gSehandle = nullptr;

    if (gSehandle != nullptr && selinux_status_updated()) {
        selabel_close(gSehandle);
        gSehandle = nullptr;
    }

    if (gSehandle == nullptr) {
        gSehandle = kIsVendor
            ? selinux_android_vendor_service_context_handle()
            : selinux_android_service_context_handle();
    }

    CHECK(gSehandle != nullptr);
    return gSehandle;
}

static int auditCallback(void *data, security_class_t /*cls*/, char *buf, size_t len) {
    const Access::CallingContext* ad = reinterpret_cast<Access::CallingContext*>(data);

    if (!ad) {
        LOG(ERROR) << "No service manager audit data";
        return 0;
    }

    snprintf(buf, len, "service=%s pid=%d uid=%d", ad->name.c_str(), ad->debugPid, ad->uid);
    return 0;
}

Access::Access() {
    union selinux_callback cb;

    cb.func_audit = auditCallback;
    selinux_set_callback(SELINUX_CB_AUDIT, cb);

    cb.func_log = kIsVendor ? selinux_vendor_log_callback : selinux_log_callback;
    selinux_set_callback(SELINUX_CB_LOG, cb);

    CHECK(selinux_status_open(true /*fallback*/) >= 0);

    CHECK(getcon(&mThisProcessContext) == 0);
}

Access::~Access() {
    freecon(mThisProcessContext);
}

Access::CallingContext Access::getCallingContext(const std::string& name) {
    IPCThreadState* ipc = IPCThreadState::self();

    const char* callingSid = ipc->getCallingSid();
    pid_t callingPid = ipc->getCallingPid();

    return CallingContext {
        .debugPid = callingPid,
        .uid = ipc->getCallingUid(),
        .sid = callingSid ? std::string(callingSid) : getPidcon(callingPid),
        .name = name,
    };
}

bool Access::canFind(const CallingContext& ctx) {
    return actionAllowedFromLookup(ctx, "find");
}

bool Access::canAdd(const CallingContext& ctx) {
    return actionAllowedFromLookup(ctx, "add");
}

bool Access::canList(const CallingContext& ctx) {
    CHECK(ctx.name == "");

    return actionAllowed(ctx, mThisProcessContext, "list");
}

bool Access::actionAllowed(const CallingContext& sctx, const char* tctx, const char* perm) {
    const char* tclass = "service_manager";

    return 0 == selinux_check_access(sctx.sid.c_str(), tctx, tclass, perm, reinterpret_cast<void*>(const_cast<CallingContext*>((&sctx))));
}

bool Access::actionAllowedFromLookup(const CallingContext& sctx, const char *perm) {
    char *tctx = nullptr;
    if (selabel_lookup(getSehandle(), &tctx, sctx.name.c_str(), 0) != 0) {
        LOG(ERROR) << "SELinux: No match for " << sctx.name << " in service_contexts.\n";
        return false;
    }

    bool allowed = actionAllowed(sctx, tctx, perm);
    freecon(tctx);
    return allowed;
}

}  // android
