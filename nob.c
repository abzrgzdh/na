#define BUILD_DIR "build"
#define TEST_DIR "tests"

#ifdef __linux__
    // The following is needed on Linux to build nob successfully when using
    // `-std=cxx -Wpedantic`, because nob uses POSIX extensions such as:
    // clock_gettime, CLOCK_MONOTONIC, nanosleep that otherwise are not
    // available without `_GNU_SOURCE`.
    //
    // See issue #125 on tsoding/nob.h GitHub repository.
    #define _GNU_SOURCE
#endif

// TODO: setup for windows.
#ifndef CC
    #define CC "cc"
#endif
// NOTE: using `-std=c2x` instead of `-std=c23` as some Linux distros as well as
// GitHub actions still do not fully support C23.

#ifndef CFLAGS
    #define CFLAGS                                                             \
        "-std=c2x", "-Wpedantic", "-Wall", "-Wextra", "-Wshadow",              \
        "-Wconversion", "-Wsign-conversion", "-Wfatal-errors", "-O0", "-g"
#endif

#ifndef CPPFLAGS
    #define CPPFLAGS "-Isrc", "-I./"
#endif

#define NOB_REBUILD_URSELF(binary_path, source_path)                           \
    CC, "-x", "c", "-std=c2x", "-O0", "-g", "-o", binary_path, source_path

#define nob_cc(cmd) nob_cmd_append(cmd, CC)
#define nob_cc_flags(cmd) nob_cmd_append(cmd, CFLAGS, CPPFLAGS)

#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#define NOB_UNSTRIP_PREFIX
#include "nob.h"

#define DEFINTE_TEST_TARGET(NAME) {                                            \
        .name = NAME,                                                          \
        .bin_path = BUILD_DIR"/"COMPONENT"/test_"NAME,                         \
        .src_path = TEST_DIR"/"COMPONENT"/test_"NAME".c" }

char const * const components[] = {"utils", "roots"};
static struct {
    char const *name;
    char const *bin_path;
    char const *src_path;
} test_targets[] = {
  #define COMPONENT "roots"
    DEFINTE_TEST_TARGET("newton_raphson"),
    DEFINTE_TEST_TARGET("bisection"),
  #undef COMPONENT
};

int build();
int build_tests();
int run_tests();
int clean_build();

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    // NOB itself
    char const* program = nob_shift(argv, argc);
    char const* subcommand = nullptr;

    if (argc == 0)
        subcommand = "build";

    char const* arg = nob_shift(argv, argc);
    int err = 0;
    if (strcmp(arg, "build") == 0) {
        err = build();
    } else if (strcmp(arg, "test") == 0) {
        err = build();
        if (err == 0)
            err = run_tests();
    } else if (strcmp(arg, "clean") == 0) {
        err = clean_build();
    } else {
        nob_log(NOB_ERROR,
                "Invalid argument passed."
                "Usage: %s [build|test|clean]\n", program);
        return 1;
    }

    return err;
}

int build()
{
    if (!nob_mkdir_if_not_exists(BUILD_DIR))
        return 1;

    return build_tests();
}

int build_tests()
{
    Nob_Cmd cmd = {};
    Nob_Procs procs = {0};

    // Make directory
    for (size_t i = 0; i < NOB_ARRAY_LEN(components); ++i) {
        char component_build_dir[256] = {};
        snprintf(component_build_dir, 256, BUILD_DIR"/%s", components[i]);
        if (!nob_mkdir_if_not_exists(component_build_dir)) return 1;
    }

    // Spawn one async process per target collecting them to procs dynamic array
    for (size_t i = 0; i < NOB_ARRAY_LEN(test_targets); ++i) {
        nob_cc(&cmd);
        nob_cc_flags(&cmd);
        nob_cc_output(&cmd, test_targets[i].bin_path);
        nob_cc_inputs(&cmd, test_targets[i].src_path);

        if (!nob_cmd_run(&cmd, .async = &procs))
            return 1;
    }

    // Wait on all the async processes to finish and reset procs dynamic array
    // to 0
    if (!nob_procs_flush(&procs))
        return 1;

    return 0;
}

int run_tests()
{
    Nob_Cmd cmd = {0};

    for (size_t i = 0; i < NOB_ARRAY_LEN(test_targets); ++i) {
        nob_cmd_append(&cmd, test_targets[i].bin_path);
        if (!nob_cmd_run(&cmd)) {
            nob_log(NOB_ERROR, "\nTests for '%s' failed.\n",
                    test_targets[i].name);

            return 1;
        }
    }

    return 0;
}

int clean_build()
{
    Nob_Cmd cmd = {};
    nob_cmd_append(&cmd, "rm", "-r", "-v", BUILD_DIR);
    if (!nob_cmd_run(&cmd)) {
        nob_log(NOB_ERROR, "\nFailed to clean the build.\n");

        return 1;
    }

    return 0;
}
