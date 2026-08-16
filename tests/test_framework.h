#ifndef TF_TEST_FRAMEWORK_H
#define TF_TEST_FRAMEWORK_H

// test_framework.h - zero-dependency test framework.
//
// Usage pattern:
//
//   static void test_something(void) {
//       TF_TEST_ASSERT(1 + 1 == 2);
//       TF_TEST_ASSERT_EQ(bst_size(t), (size_t)3);
//   }
//
//   int main(void) {
//       TF_TEST_SUITE_BEGIN("NEWTON");
//       TF_RUN_TEST(test_something);
//       TF_TEST_SUITE_END();
//   }
//
// NOTE: This file MUST come before any #include, particularly not before
// "na.h", <stdlib.h> and <limits.h>. If this is not possible, when building
// include this macro in the compiler options: -D_XOPEN_SOURCE=500.
//
//     NOTE: Without `-D_XOPEN_SOURCE=500`, the declaration of realpath() will
//     be skipped when including stdlib.h.  From there, for historical reasons,
//     the compiler assumes realpth() has the following definition
//
//         int realpath()
//     or
//         int realpath(int arg1, int arg2, int arg3, ...)
//
//     source:
//
//         https://github.com/gpakosz/whereami/issues/33#issuecomment-1019284523
//
// TODO: Simple logging facility for readable output

// Platform detection and feature macros.
#if defined(__linux__)
    // To have `realpath` on Linux
    // TODO: Explain why this is really needed.
    // NOTE: See the comment above on this macro and `man 3 realpath`.
    #ifndef _XOPEN_SOURCE
        #define _XOPEN_SOURCE 500
    #endif
#endif

// Now include headers
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

// Windows compatibility for realpath
#ifdef _WIN32
    #include <windows.h>
    #define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
    #ifndef PATH_MAX
        #define PATH_MAX _MAX_PATH
    #endif
#endif

typedef int32_t TF_Int;

// -- Internal counters ------------------------------------------------------

[[maybe_unused]] static TF_Int _TF_run    = 0;
[[maybe_unused]] static TF_Int _TF_passed = 0;
[[maybe_unused]] static TF_Int _TF_failed = 0;

// -- File path --------------------------------------------------------------

#define MAX_ABSOLUTE_PATH_LENGTH 4096

[[maybe_unused]] static char const *
_TF_absolute_path(char const *file)
{
    static char path[MAX_ABSOLUTE_PATH_LENGTH];

    if (realpath(file, path))
        return path;

    return file;
}

// -- Core assert -------------------------------------------------------------

#define TF_TEST_ASSERT(expr)                                                   \
    do {                                                                       \
        _TF_run++;                                                             \
        if (expr) {                                                            \
            _TF_passed++;                                                      \
            printf("    PASS: %s\n"                                            \
                   "          in %s()\n",                                      \
                   #expr, __func__);                                           \
        } else {                                                               \
            _TF_failed++;                                                      \
            printf("    FAIL: %s\n"                                            \
                   "          in %s()\n"                                       \
                   "          at %s:%d\n",                                     \
                   #expr, __func__,                                            \
                   _TF_absolute_path(__FILE__), __LINE__);                     \
        }                                                                      \
    } while (0)

// -- Derived assert macros ---------------------------------------------------

#define TF_TEST_ASSERT_EQ(a, b)                                                \
    TF_TEST_ASSERT((a) == (b))

#define TF_TEST_ASSERT_NEQ(a, b)                                               \
    TF_TEST_ASSERT((a) != (b))

#define TF_TEST_ASSERT_NULL(p)                                                 \
    TF_TEST_ASSERT((p) == nullptr)

#define TF_TEST_ASSERT_NOT_NULL(p)                                             \
    TF_TEST_ASSERT((p) != nullptr)

#define TF_TEST_ASSERT_STR_EQ(a, b)                                            \
    TF_TEST_ASSERT(strcmp((a), (b)) == 0)

#define TF_TEST_ASSERT_GT(a, b)                                                \
    TF_TEST_ASSERT((a) > (b))

#define TF_TEST_ASSERT_GTE(a, b)                                               \
    TF_TEST_ASSERT((a) >= (b))

#define TF_TEST_ASSERT_LT(a, b)                                                \
    TF_TEST_ASSERT((a) < (b))

#define TF_TEST_ASSERT_LTE(a, b)                                               \
    TF_TEST_ASSERT((a) <= (b))

// A tolerance for equality, i.e., if (fabs(a - b) < tol) then a and b are
// equal.
#define TF_TEST_SCALAR_TOLERANCE DBL_TRUE_MIN

#define TF_TEST_ASSERT_SCALAR_EQ(a, b)                                         \
    TF_TEST_ASSERT(fabs((a) - (b)) <= TF_TEST_SCALAR_TOLERANCE)

#define TF_TEST_ASSERT_SCALAR_NEQ(a, b)                                        \
    TF_TEST_ASSERT(fabs((a) - (b)) > TF_TEST_SCALAR_TOLERANCE)

// -- Test runner -------------------------------------------------------------

#define TF_RUN_TEST(fn)                                                        \
    do {                                                                       \
        printf("  TEST: %s\n", #fn);                                           \
        fn();                                                                  \
    } while (0)

// -- Suite boundaries --------------------------------------------------------

#define TF_TEST_SUITE_BEGIN(name)                                              \
    printf("\n== %s ==\n", name)

#define TF_TEST_SUITE_END()                                                    \
    do {                                                                       \
        printf("\n%d run, %d passed, %d failed\n",                             \
               _TF_run, _TF_passed, _TF_failed);                               \
        return (_TF_failed > 0) ? 1 : 0;                                       \
    } while (0)

#endif // TF_TEST_FRAMEWORK_H
