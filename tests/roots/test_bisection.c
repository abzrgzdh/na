// NOTE: Must be included before any other header file.
#include "../test_framework.h"

#define NA_IMPLEMENTATION
#include "na.h"

// -------------------------------------------------------------------------
// Test functions
// -------------------------------------------------------------------------

DEFINE_FUNCR1R1(f_cubic, x, ((x * x * x) + x - 1.0))

// f(x) = x - 1, root at x = 1.
DEFINE_FUNCR1R1(f_linear, x, (x - 1.0))

// Root at the left endpoint when called with [0, 1].
DEFINE_FUNCR1R1(f_root_left, x, (x))

// Root at the right endpoint when called with [0, 1].
DEFINE_FUNCR1R1(f_root_right, x, (x - 1.0))

// No root in [0, 1].
DEFINE_FUNCR1R1(f_no_bracket, x, (x + 1.0))

// Always returns NaN.
DEFINE_FUNCR1R1(f_nan, _, (NA_SCALAR_NAN))

// The root is extremely close to the left endpoint.
//
// This allows us to test convergence based on the bracket width rather than
// relying on the residual becoming sufficiently small first.
DEFINE_FUNCR1R1(f_small_root, x, (x - 1e-8))


// -------------------------------------------------------------------------
// Tests
// -------------------------------------------------------------------------

static void
test_converges_to_root(void)
{
    NA_SolverOption option = {
        .abs_bracket_tolerance = 1e-12,
        .rel_bracket_tolerance = 1e-12,
        .residual_tolerance = 1e-12,
        .max_iterations = 100,
    };

    NA_RootReport report = NA_bisection(f_cubic, 0.0, 1.0, option);

    const NA_Scalar exact_root = 0.6823278038280193;
    const NA_Scalar abs_error  = fabs(exact_root - report.root);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_SUCCESS);
    TF_TEST_ASSERT(abs_error < option.abs_bracket_tolerance);
    TF_TEST_ASSERT(report.residual < option.residual_tolerance);
    TF_TEST_ASSERT_GT(report.iterations, 0);
    TF_TEST_ASSERT_GT(report.function_evaluations, 0);
}


static void
test_root_at_left_endpoint(void)
{
    NA_SolverOption option = {
        .abs_bracket_tolerance = 1e-12,
        .rel_bracket_tolerance = 1e-12,
        .residual_tolerance = 1e-12,
        .max_iterations = 100,
    };

    NA_RootReport report = NA_bisection(f_root_left, 0.0, 1.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_SUCCESS);
    TF_TEST_ASSERT_EQ(report.root, 0.0);
    TF_TEST_ASSERT_EQ(report.residual, 0.0);
    TF_TEST_ASSERT_EQ(report.iterations, 0);
    TF_TEST_ASSERT_EQ(report.function_evaluations, 2);
}


static void
test_root_at_right_endpoint(void)
{
    NA_SolverOption option = {
        .abs_bracket_tolerance = 1e-12,
        .rel_bracket_tolerance = 1e-12,
        .residual_tolerance = 1e-12,
        .max_iterations = 100,
    };

    NA_RootReport report = NA_bisection(f_root_right, 0.0, 1.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_SUCCESS);
    TF_TEST_ASSERT_EQ(report.root, 1.0);
    TF_TEST_ASSERT_EQ(report.residual, 0.0);
    TF_TEST_ASSERT_EQ(report.iterations, 0);
    TF_TEST_ASSERT_EQ(report.function_evaluations, 2);
}


static void
test_invalid_bracket(void)
{
    NA_SolverOption option = {
        .abs_bracket_tolerance = 1e-12,
        .rel_bracket_tolerance = 1e-12,
        .residual_tolerance = 1e-12,
        .max_iterations = 100,
    };

    NA_RootReport report = NA_bisection(f_no_bracket, 0.0, 1.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_INVALID_BRACKET);
}


static void
test_non_finite_function_evaluation(void)
{
    NA_SolverOption option = {
        .abs_bracket_tolerance = 1e-12,
        .rel_bracket_tolerance = 1e-12,
        .residual_tolerance = 1e-12,
        .max_iterations = 100,
    };

    NA_RootReport report = NA_bisection(f_nan, 0.0, 1.0, option);

    TF_TEST_ASSERT_EQ(
        report.error,
        NA_ERROR_ROOT_NONFINITE_FUNCTION_EVALUATION
    );

    TF_TEST_ASSERT_EQ(report.function_evaluations, 2);
}


static void
test_residual_convergence(void)
{
    NA_SolverOption option = {
        .abs_bracket_tolerance = 0.0,
        .rel_bracket_tolerance = 0.0,
        .residual_tolerance = 1e-12,
        .max_iterations = 100,
    };

    NA_RootReport report = NA_bisection(f_linear, 0.0, 2.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_SUCCESS);
    TF_TEST_ASSERT_LT(report.residual, option.residual_tolerance);
    TF_TEST_ASSERT_GT(report.iterations, 0);
}


static void
test_bracket_convergence(void)
{
    NA_SolverOption option = {
        .abs_bracket_tolerance = 1e-6,
        .rel_bracket_tolerance = 0.0,
        .residual_tolerance = 0.0,
        .max_iterations = 100,
    };

    NA_RootReport report = NA_bisection(f_small_root, 0.0, 1.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_SUCCESS);
    TF_TEST_ASSERT_LT(report.bracket, option.abs_bracket_tolerance);
    TF_TEST_ASSERT_GT(report.iterations, 0);
}


static void
test_max_iterations(void)
{
    NA_SolverOption option = {
        .abs_bracket_tolerance = 0.0,
        .rel_bracket_tolerance = 0.0,
        .residual_tolerance = 0.0,
        .max_iterations = 1,
    };

    NA_RootReport report = NA_bisection(f_cubic, 0.0, 1.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_MAX_ITERATIONS);
    TF_TEST_ASSERT_EQ(report.iterations, 1);
    TF_TEST_ASSERT_EQ(report.function_evaluations, 3);
}


static void
test_invalid_function_pointer(void)
{
    NA_SolverOption option = {
        .abs_bracket_tolerance = 1e-12,
        .rel_bracket_tolerance = 1e-12,
        .residual_tolerance = 1e-12,
        .max_iterations = 100,
    };

    NA_RootReport report = NA_bisection(NULL, 0.0, 1.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_INVALID_FUNC_POINTER);
}


static void
test_non_finite_bracket(void)
{
    NA_SolverOption option = {
        .abs_bracket_tolerance = 1e-12,
        .rel_bracket_tolerance = 1e-12,
        .residual_tolerance = 1e-12,
        .max_iterations = 100,
    };

    NA_RootReport report = NA_bisection(
        f_linear,
        NA_SCALAR_NAN,
        1.0,
        option
    );

    TF_TEST_ASSERT_EQ(
        report.error,
        NA_ERROR_ROOT_NONFINITE_BRACKET_RANGE
    );
}


static void
test_invalid_solver_option(void)
{
    NA_SolverOption option = {
        .abs_bracket_tolerance = -1.0,
        .rel_bracket_tolerance = 1e-12,
        .residual_tolerance = 1e-12,
        .max_iterations = 100,
    };

    NA_RootReport report = NA_bisection(f_cubic, 0.0, 1.0, option);

    printf("  === report.error = %s\n", NA_RootError_get_message(report.error));

    TF_TEST_ASSERT_EQ(
        report.error,
        NA_ERROR_ROOT_INVALID_SOLVER_OPTION
    );
}


/* ------------------------------------------------------------------------- */
/* Test suite                                                                */
/* ------------------------------------------------------------------------- */

int
main(void)
{
    TF_TEST_SUITE_BEGIN("Bisection");

    TF_RUN_TEST(test_converges_to_root);
    TF_RUN_TEST(test_root_at_left_endpoint);
    TF_RUN_TEST(test_root_at_right_endpoint);

    TF_RUN_TEST(test_invalid_bracket);
    TF_RUN_TEST(test_invalid_function_pointer);
    TF_RUN_TEST(test_non_finite_bracket);
    TF_RUN_TEST(test_non_finite_function_evaluation);
    TF_RUN_TEST(test_invalid_solver_option);

    TF_RUN_TEST(test_residual_convergence);
    TF_RUN_TEST(test_bracket_convergence);
    TF_RUN_TEST(test_max_iterations);

    TF_TEST_SUITE_END();
}
