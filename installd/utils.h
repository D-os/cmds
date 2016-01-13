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

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <vector>

#include <dirent.h>
#include <inttypes.h>
#include <unistd.h>
#include <utime.h>

#include <cutils/multiuser.h>

#include <installd_constants.h>

namespace android {
namespace installd {

struct dir_rec_t;

typedef struct cache_dir_struct {
    struct cache_dir_struct* parent;
    int32_t childCount;
    int32_t hiddenCount;
    int32_t deleted;
    char name[];
} cache_dir_t;

typedef struct {
    cache_dir_t* dir;
    time_t modTime;
    char name[];
} cache_file_t;

typedef struct {
    size_t numDirs;
    size_t availDirs;
    cache_dir_t** dirs;
    size_t numFiles;
    size_t availFiles;
    cache_file_t** files;
    size_t numCollected;
    void* memBlocks;
    int8_t* curMemBlockAvail;
    int8_t* curMemBlockEnd;
} cache_t;

int create_pkg_path(char path[PKG_PATH_MAX],
                    const char *pkgname,
                    const char *postfix,
                    userid_t userid);

std::string create_data_path(const char* volume_uuid);

std::string create_data_app_path(const char* volume_uuid);

std::string create_data_app_package_path(const char* volume_uuid, const char* package_name);

// TODO: finish refactoring to "_ce"
std::string create_data_user_path(const char* volume_uuid, userid_t userid);
std::string create_data_user_de_path(const char* volume_uuid, userid_t userid);

std::string create_data_user_package_path(const char* volume_uuid,
        userid_t user, const char* package_name);
std::string create_data_user_de_package_path(const char* volume_uuid,
        userid_t user, const char* package_name);

std::string create_data_media_path(const char* volume_uuid, userid_t userid);

std::vector<userid_t> get_known_users(const char* volume_uuid);

int create_user_config_path(char path[PKG_PATH_MAX], userid_t userid);

int create_move_path(char path[PKG_PATH_MAX],
                     const char* pkgname,
                     const char* leaf,
                     userid_t userid);

int is_valid_package_name(const char* pkgname);

int delete_dir_contents(const std::string& pathname);
int delete_dir_contents_and_dir(const std::string& pathname);

int delete_dir_contents(const char *pathname,
                        int also_delete_dir,
                        int (*exclusion_predicate)(const char *name, const int is_dir));

int delete_dir_contents_fd(int dfd, const char *name);

int copy_dir_files(const char *srcname, const char *dstname, uid_t owner, gid_t group);

int lookup_media_dir(char basepath[PATH_MAX], const char *dir);

int64_t data_disk_free(const std::string& data_path);

cache_t* start_cache_collection();

void add_cache_files(cache_t* cache, const char *basepath, const char *cachedir);

void clear_cache_files(const std::string& data_path, cache_t* cache, int64_t free_size);

void finish_cache_collection(cache_t* cache);

int validate_system_app_path(const char* path);

int get_path_from_env(dir_rec_t* rec, const char* var);

int get_path_from_string(dir_rec_t* rec, const char* path);

int copy_and_append(dir_rec_t* dst, const dir_rec_t* src, const char* suffix);

int validate_apk_path(const char *path);
int validate_apk_path_subdirs(const char *path);

int append_and_increment(char** dst, const char* src, size_t* dst_size);

char *build_string2(const char *s1, const char *s2);
char *build_string3(const char *s1, const char *s2, const char *s3);

int ensure_dir(const char* path, mode_t mode, uid_t uid, gid_t gid);
int ensure_config_user_dirs(userid_t userid);

int wait_child(pid_t pid);

}  // namespace installd
}  // namespace android

#endif  // UTILS_H_
