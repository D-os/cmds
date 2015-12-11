/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef _DUMPSTATE_H_
#define _DUMPSTATE_H_

/* When defined, skips the real dumps and just print the section headers.
   Useful when debugging dumpstate itself. */
//#define _DUMPSTATE_DRY_RUN_

#ifdef _DUMPSTATE_DRY_RUN_
#define ON_DRY_RUN_RETURN(X) return X
#define ON_DRY_RUN(code) code
#else
#define ON_DRY_RUN_RETURN(X)
#define ON_DRY_RUN(code)
#endif


#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <vector>

#define SU_PATH "/system/xbin/su"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (for_each_pid_func)(int, const char *);
typedef void (for_each_tid_func)(int, int, const char *);

/* Estimated total weight of bugreport generation.
 *
 * Each section contributes to the total weight by an individual weight, so the overall progress
 * can be calculated by dividing the all completed weight by the total weight.
 *
 * This value is defined empirically and it need to be adjusted as more sections are added.
 *
 * It does not need to match the exact sum of all sections, but ideally it should to be slight more
 * than such sum: a value too high will cause the bugreport to finish before the user expected (for
 * example, jumping from 70% to 100%), while a value too low will cause the progress to fluctuate
 * down (for example, from 70% to 50%, since the actual max will be automatically increased every
 * time it is reached).
 */
static const int WEIGHT_TOTAL = 4000;

/* Most simple commands have 10 as timeout, so 5 is a good estimate */
static const int WEIGHT_FILE = 5;

/*
 * TOOD: the dumpstate internal state is getting fragile; for example, this variable is defined
 * here, declared at utils.cpp, and used on utils.cpp and dumpstate.cpp.
 * It would be better to take advantage of the C++ migration and encapsulate the state in an object,
 * but that will be better handled in a major C++ refactoring, which would also get rid of other C
 * idioms (like using std::string instead of char*, removing varargs, etc...) */
extern int do_update_progress, progress, weight_total;

/* prints the contents of a file */
int dump_file(const char *title, const char *path);

/* prints the contents of the fd
 * fd must have been opened with the flag O_NONBLOCK.
 */
int dump_file_from_fd(const char *title, const char *path, int fd);

/* calls skip to gate calling dump_from_fd recursively
 * in the specified directory. dump_from_fd defaults to
 * dump_file_from_fd above when set to NULL. skip defaults
 * to false when set to NULL. dump_from_fd will always be
 * called with title NULL.
 */
int dump_files(const char *title, const char *dir,
        bool (*skip)(const char *path),
        int (*dump_from_fd)(const char *title, const char *path, int fd));

/* forks a command and waits for it to finish -- terminate args with NULL */
int run_command(const char *title, int timeout_seconds, const char *command, ...);

/* forks a command and waits for it to finish
   first element of args is the command, and last must be NULL.
   command is always ran, even when _DUMPSTATE_DRY_RUN_ is defined. */
int run_command_always(const char *title, int timeout_seconds, const char *args[]);

/* sends a broadcast using Activity Manager */
void send_broadcast(const std::string& action, const std::vector<std::string>& args);

/* updates the overall progress of dumpstate by the given weight increment */
void update_progress(int weight);

/* prints all the system properties */
void print_properties();

/* redirect output to a service control socket */
void redirect_to_socket(FILE *redirect, const char *service);

/* redirect output to a file */
void redirect_to_file(FILE *redirect, char *path);

/* dump Dalvik and native stack traces, return the trace file location (NULL if none) */
const char *dump_traces();

/* for each process in the system, run the specified function */
void for_each_pid(for_each_pid_func func, const char *header);

/* for each thread in the system, run the specified function */
void for_each_tid(for_each_tid_func func, const char *header);

/* Displays a blocked processes in-kernel wait channel */
void show_wchan(int pid, int tid, const char *name);

/* Runs "showmap" for a process */
void do_showmap(int pid, const char *name);

/* Gets the dmesg output for the kernel */
void do_dmesg();

/* Prints the contents of all the routing tables, both IPv4 and IPv6. */
void dump_route_tables();

/* Play a sound via Stagefright */
void play_sound(const char *path);

/* Implemented by libdumpstate_board to dump board-specific info */
void dumpstate_board();

/* Takes a screenshot and save it to the given file */
void take_screenshot(const std::string& path);

#ifdef __cplusplus
}
#endif

/* dump eMMC Extended CSD data */
void dump_emmc_ecsd(const char *ext_csd_path);

#endif /* _DUMPSTATE_H_ */
