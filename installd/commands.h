/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <inttypes.h>
#include <unistd.h>

#include <vector>

#include <binder/BinderService.h>
#include <cutils/multiuser.h>

#include "android/os/BnInstalld.h"
#include "installd_constants.h"

namespace android {
namespace installd {

class InstalldNativeService : public BinderService<InstalldNativeService>, public os::BnInstalld {
public:
    static status_t start();
    static char const* getServiceName() { return "installd"; }
    virtual status_t dump(int fd, const Vector<String16> &args) override;

    binder::Status createUserData(const std::unique_ptr<std::string>& uuid, int32_t userId,
            int32_t userSerial, int32_t flags);
    binder::Status destroyUserData(const std::unique_ptr<std::string>& uuid, int32_t userId,
            int32_t flags);

    binder::Status createAppData(const std::unique_ptr<std::string>& uuid,
            const std::string& packageName, int32_t userId, int32_t flags, int32_t appId,
            const std::string& seInfo, int32_t targetSdkVersion);
    binder::Status restoreconAppData(const std::unique_ptr<std::string>& uuid,
            const std::string& packageName, int32_t userId, int32_t flags, int32_t appId,
            const std::string& seInfo);
    binder::Status migrateAppData(const std::unique_ptr<std::string>& uuid,
            const std::string& packageName, int32_t userId, int32_t flags);
    binder::Status clearAppData(const std::unique_ptr<std::string>& uuid,
            const std::string& packageName, int32_t userId, int32_t flags, int64_t ceDataInode);
    binder::Status destroyAppData(const std::unique_ptr<std::string>& uuid,
            const std::string& packageName, int32_t userId, int32_t flags, int64_t ceDataInode);
    binder::Status getAppDataInode(const std::unique_ptr<std::string>& uuid,
            const std::string& packageName, int32_t userId, int32_t flags, int64_t* _aidl_return);

    binder::Status moveCompleteApp(const std::unique_ptr<std::string>& fromUuid,
            const std::unique_ptr<std::string>& toUuid, const std::string& packageName,
            const std::string& dataAppName, int32_t appId, const std::string& seInfo,
            int32_t targetSdkVersion);

    binder::Status rmdex(const std::string& codePath, const std::string& instructionSet);

    binder::Status mergeProfiles(int32_t uid, const std::string& packageName, bool* _aidl_return);
    binder::Status dumpProfiles(int32_t uid, const std::string& packageName,
            const std::string& codePaths, bool* _aidl_return);
    binder::Status clearAppProfiles(const std::string& packageName);
    binder::Status destroyAppProfiles(const std::string& packageName);

    binder::Status idmap(const std::string& targetApkPath, const std::string& overlayApkPath, int32_t uid);
    binder::Status rmPackageDir(const std::string& packageDir);
    binder::Status markBootComplete(const std::string& instructionSet);
    binder::Status freeCache(const std::unique_ptr<std::string>& uuid, int64_t freeStorageSize);
    binder::Status linkNativeLibraryDirectory(const std::unique_ptr<std::string>& uuid, const std::string& packageName, const std::string& nativeLibPath32, int32_t userId);
    binder::Status createOatDir(const std::string& oatDir, const std::string& instructionSet);
    binder::Status linkFile(const std::string& relativePath, const std::string& fromBase, const std::string& toBase);
    binder::Status moveAb(const std::string& apkPath, const std::string& instructionSet, const std::string& outputPath);
    binder::Status deleteOdex(const std::string& apkPath, const std::string& instructionSet, const std::string& outputPath);
};

int get_app_size(const char *uuid, const char *pkgname, int userid, int flags, ino_t ce_data_inode,
        const char* code_path, int64_t *codesize, int64_t *datasize, int64_t *cachesize,
        int64_t *asecsize);

int dexopt(const char *apk_path,
           uid_t uid,
           const char *pkgName,
           const char *instruction_set,
           int dexopt_needed,
           const char* oat_dir,
           int dexopt_flags,
           const char* compiler_filter,
           const char* volume_uuid,
           const char* shared_libraries);
static_assert(DEXOPT_PARAM_COUNT == 10U, "Unexpected dexopt param size");

// Helper for the above, converting arguments.
int dexopt(const char* const params[DEXOPT_PARAM_COUNT]);

}  // namespace installd
}  // namespace android

#endif  // COMMANDS_H_
