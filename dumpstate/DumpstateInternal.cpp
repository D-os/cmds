/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "dumpstate"

#include "DumpstateInternal.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdint>
#include <string>
#include <vector>

#include <android-base/file.h>
#include <cutils/log.h>
#include <private/android_filesystem_config.h>

uint64_t Nanotime() {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec * NANOS_PER_SEC + ts.tv_nsec);
}

// Switches to non-root user and group.
bool DropRootUser() {
    if (getgid() == AID_SHELL && getuid() == AID_SHELL) {
        MYLOGD("drop_root_user(): already running as Shell\n");
        return true;
    }
    /* ensure we will keep capabilities when we drop root */
    if (prctl(PR_SET_KEEPCAPS, 1) < 0) {
        MYLOGE("prctl(PR_SET_KEEPCAPS) failed: %s\n", strerror(errno));
        return false;
    }

    gid_t groups[] = {AID_LOG,  AID_SDCARD_R,     AID_SDCARD_RW, AID_MOUNT,
                      AID_INET, AID_NET_BW_STATS, AID_READPROC,  AID_BLUETOOTH};
    if (setgroups(sizeof(groups) / sizeof(groups[0]), groups) != 0) {
        MYLOGE("Unable to setgroups, aborting: %s\n", strerror(errno));
        return false;
    }
    if (setgid(AID_SHELL) != 0) {
        MYLOGE("Unable to setgid, aborting: %s\n", strerror(errno));
        return false;
    }
    if (setuid(AID_SHELL) != 0) {
        MYLOGE("Unable to setuid, aborting: %s\n", strerror(errno));
        return false;
    }

    struct __user_cap_header_struct capheader;
    struct __user_cap_data_struct capdata[2];
    memset(&capheader, 0, sizeof(capheader));
    memset(&capdata, 0, sizeof(capdata));
    capheader.version = _LINUX_CAPABILITY_VERSION_3;
    capheader.pid = 0;

    capdata[CAP_TO_INDEX(CAP_SYSLOG)].permitted = CAP_TO_MASK(CAP_SYSLOG);
    capdata[CAP_TO_INDEX(CAP_SYSLOG)].effective = CAP_TO_MASK(CAP_SYSLOG);
    capdata[0].inheritable = 0;
    capdata[1].inheritable = 0;

    if (capset(&capheader, &capdata[0]) < 0) {
        MYLOGE("capset failed: %s\n", strerror(errno));
        return false;
    }

    return true;
}

int DumpFileFromFdToFd(const std::string& title, const std::string& path_string, int fd, int out_fd,
                       bool dry_run) {
    const char* path = path_string.c_str();
    if (!title.empty()) {
        dprintf(out_fd, "------ %s (%s", title.c_str(), path);

        struct stat st;
        // Only show the modification time of non-device files.
        size_t path_len = strlen(path);
        if ((path_len < 6 || memcmp(path, "/proc/", 6)) &&
            (path_len < 5 || memcmp(path, "/sys/", 5)) &&
            (path_len < 3 || memcmp(path, "/d/", 3)) && !fstat(fd, &st)) {
            char stamp[80];
            time_t mtime = st.st_mtime;
            strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", localtime(&mtime));
            dprintf(out_fd, ": %s", stamp);
        }
        dprintf(out_fd, ") ------\n");
        fsync(out_fd);
    }
    if (dry_run) {
        if (out_fd != STDOUT_FILENO) {
            // There is no title, but we should still print a dry-run message
            dprintf(out_fd, "%s: skipped on dry run\n", path);
        } else if (!title.empty()) {
            dprintf(out_fd, "\t(skipped on dry run)\n");
        }
        fsync(out_fd);
        return 0;
    }
    bool newline = false;
    fd_set read_set;
    timeval tm;
    while (true) {
        FD_ZERO(&read_set);
        FD_SET(fd, &read_set);
        /* Timeout if no data is read for 30 seconds. */
        tm.tv_sec = 30;
        tm.tv_usec = 0;
        uint64_t elapsed = Nanotime();
        int ret = TEMP_FAILURE_RETRY(select(fd + 1, &read_set, nullptr, nullptr, &tm));
        if (ret == -1) {
            dprintf(out_fd, "*** %s: select failed: %s\n", path, strerror(errno));
            newline = true;
            break;
        } else if (ret == 0) {
            elapsed = Nanotime() - elapsed;
            dprintf(out_fd, "*** %s: Timed out after %.3fs\n", path, (float)elapsed / NANOS_PER_SEC);
            newline = true;
            break;
        } else {
            char buffer[65536];
            ssize_t bytes_read = TEMP_FAILURE_RETRY(read(fd, buffer, sizeof(buffer)));
            if (bytes_read > 0) {
                android::base::WriteFully(out_fd, buffer, bytes_read);
                newline = (buffer[bytes_read - 1] == '\n');
            } else {
                if (bytes_read == -1) {
                    dprintf(out_fd, "*** %s: Failed to read from fd: %s", path, strerror(errno));
                    newline = true;
                }
                break;
            }
        }
    }
    close(fd);

    if (!newline) dprintf(out_fd, "\n");
    if (!title.empty()) dprintf(out_fd, "\n");
    return 0;
}
