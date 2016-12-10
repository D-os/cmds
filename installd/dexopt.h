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

#ifndef DEXOPT_H_
#define DEXOPT_H_

#include <sys/types.h>

namespace android {
namespace installd {

int dexopt(const char *apk_path, uid_t uid, const char *pkgName, const char *instruction_set,
        int dexopt_needed, const char* oat_dir, int dexopt_flags, const char* compiler_filter,
        const char* volume_uuid, const char* shared_libraries);

static constexpr size_t DEXOPT_PARAM_COUNT = 10U;
static_assert(DEXOPT_PARAM_COUNT == 10U, "Unexpected dexopt param size");

// Helper for the above, converting arguments.
int dexopt(const char* const params[DEXOPT_PARAM_COUNT]);

}  // namespace installd
}  // namespace android

#endif  // DEXOPT_H_
