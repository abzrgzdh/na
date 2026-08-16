// NOTE: Must be included before any other header file.
#include "../test_framework.h"

#define NA_IMPLEMENTATION
#include "na.h"

// -------------------------------------------------------------------------
// Test functions
// -------------------------------------------------------------------------

DEFINE_FUNCR1R1(f_cubic, x, ((x * x * x) + x - 1.0))
DEFINE_FUNCR1R1(df_cubic, x, ((3.0 * x * x) + 1.0))

/* f(x) = x - 1, root at x = 1. */
DEFINE_FUNCR1R1(f_linear, x, (x - 1.0))
DEFINE_FUNCR1R1(df_linear, _, (1.0))

/*
 * The Newton step from x = 0 is -1e-12.
 *
 * This is useful for testing convergence through the step-size criterion
 * while the residual is still much larger than its tolerance.
 */
DEFINE_FUNCR1R1(f_small_step, x, (x + 1e-12))
DEFINE_FUNCR1R1(df_small_step, _, (1.0))

/* f'(x) = 0 everywhere. */
static NA_Scalar
f_zero_derivative(NA_Scalar x)
{
    return x * x + 1.0;
}

static NA_Scalar
df_zero_derivative(NA_Scalar x)
{
    (void)x;
    return 0.0;
}


/* Return NaN from the function evaluation. */
static NA_Scalar
f_nan(NA_Scalar x)
{
    (void)x;
    return NA_SCALAR_NAN;
}

static NA_Scalar
df_nan_function(NA_Scalar x)
{
    (void)x;
    return 1.0;
}


/* Return NaN from the derivative evaluation. */
static NA_Scalar
f_nan_derivative(NA_Scalar x)
{
    return x - 1.0;
}

static NA_Scalar
df_nan_derivative(NA_Scalar x)
{
    (void)x;
    return NA_SCALAR_NAN;
}


/*
 * Newton's method diverges for this starting point:
 *
 *     f(x)  = x^3 - 2x + 2
 *     f'(x) = 3x^2 - 2
 *
 * with x0 = 0, producing the well-known 0 -> 1 -> 0 cycle.
 */
static NA_Scalar
f_max_iterations(NA_Scalar x)
{
    return (x * x * x) - (2.0 * x) + 2.0;
}

static NA_Scalar
df_max_iterations(NA_Scalar x)
{
    return (3.0 * x * x) - 2.0;
}


/* ------------------------------------------------------------------------- */
/* Tests                                                                      */
/* ------------------------------------------------------------------------- */

static void
test_converges_to_root(void)
{
    NA_SolverOption option = {
        .abs_step_tolerance = 1e-12,
        .rel_step_tolerance = 1e-12,
        .residual_tolerance = 1e-12,
        .derivative_tolerance = 1e-14,
        .max_iterations = 100,
    };

    NA_RootReport report =
        NA_newton_raphson(f_cubic, df_cubic, 1.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_SUCCESS);
    TF_TEST_ASSERT(fabs(report.root - 0.6823278038280193) < 1e-12);
    TF_TEST_ASSERT(report.residual < 1e-12);
    TF_TEST_ASSERT(report.step != 0.0);
    TF_TEST_ASSERT_GT(report.iterations, 0);
    TF_TEST_ASSERT_GT(report.function_evaluations, 0);
    TF_TEST_ASSERT_GT(report.derivative_evaluations, 0);
}

static void
test_residual_convergence_at_initial_value(void)
{
    NA_SolverOption option = {
        .residual_tolerance = 1e-12,
        .max_iterations = 100,
    };

    NA_RootReport report =
        NA_newton_raphson(f_linear, df_linear, 1.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_SUCCESS);
    TF_TEST_ASSERT_EQ(report.root, 1.0);
    TF_TEST_ASSERT_EQ(report.iterations, 0);
    TF_TEST_ASSERT_EQ(report.function_evaluations, (NA_Size)1);
    TF_TEST_ASSERT_EQ(report.derivative_evaluations, (NA_Size)0);
    TF_TEST_ASSERT_EQ(report.step, 0.0);
    TF_TEST_ASSERT(report.residual < 1e-12);
}


static void
test_step_convergence(void)
{
    NA_SolverOption option = {
        .abs_step_tolerance = 1e-10,
        .rel_step_tolerance = 0.0,
        .residual_tolerance = 1e-15,
        .derivative_tolerance = 1e-15,
        .max_iterations = 100,
    };

    NA_RootReport report =
        NA_newton_raphson(f_small_step, df_small_step, 0.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_SUCCESS);
    TF_TEST_ASSERT_EQ(report.iterations, (NA_Size)1);
    TF_TEST_ASSERT(fabs(report.root + 1e-12) < 1e-24);
    TF_TEST_ASSERT(fabs(report.step + 1e-12) < 1e-24);

    /*
     * The residual is deliberately larger than its tolerance. Therefore
     * success must have occurred because of the step-size criterion.
     */
    TF_TEST_ASSERT(report.residual > option.residual_tolerance);
}


static void
test_default_options(void)
{
    /*
     * Aggregate initialization is intentional: unspecified members are
     * initialized to zero and NA_newton_raphson() replaces those values
     * with the solver defaults.
     */
    NA_SolverOption option = {
        .residual_tolerance = 1e-12,
    };

    NA_RootReport report =
        NA_newton_raphson(f_cubic, df_cubic, 1.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_SUCCESS);
    TF_TEST_ASSERT(report.residual < 1e-12);
}


static void
test_invalid_function_pointer(void)
{
    NA_SolverOption option = {
        .max_iterations = 100,
    };

    NA_RootReport report =
        NA_newton_raphson(nullptr, df_cubic, 1.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_INVALID_FUNC_POINTER);
}


static void
test_invalid_derivative_function_pointer(void)
{
    NA_SolverOption option = {
        .max_iterations = 100,
    };

    NA_RootReport report =
        NA_newton_raphson(f_cubic, nullptr, 1.0, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_INVALID_FUNC_POINTER);
}


static void
test_nonfinite_initial_value(void)
{
    NA_SolverOption option = {
        .max_iterations = 100,
    };

    NA_RootReport report =
        NA_newton_raphson(f_cubic, df_cubic, NA_SCALAR_NAN, option);

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_NONFINITE_INITIAL_VALUE);
}


static void
test_nonfinite_function_evaluation(void)
{
    NA_SolverOption option = {
        .max_iterations = 100,
    };

    NA_RootReport report =
        NA_newton_raphson(f_nan, df_nan_function, 1.0, option);

    TF_TEST_ASSERT_EQ(
        report.error,
        NA_ERROR_ROOT_NONFINITE_FUNCTION_EVALUATION
    );

    TF_TEST_ASSERT_EQ(report.function_evaluations, (NA_Size)1);
    TF_TEST_ASSERT_EQ(report.derivative_evaluations, (NA_Size)0);
    TF_TEST_ASSERT(isnan(report.residual));
}


static void
test_nonfinite_derivative_evaluation(void)
{
    NA_SolverOption option = {
        .max_iterations = 100,
    };

    NA_RootReport report =
        NA_newton_raphson(
            f_nan_derivative,
            df_nan_derivative,
            0.0,
            option
        );

    TF_TEST_ASSERT_EQ(
        report.error,
        NA_ERROR_ROOT_NONFINITE_DERIVATIVE_EVALUATION
    );

    TF_TEST_ASSERT_EQ(report.function_evaluations, (NA_Size)1);
    TF_TEST_ASSERT_EQ(report.derivative_evaluations, (NA_Size)1);
    TF_TEST_ASSERT_EQ(report.root, 0.0);
    TF_TEST_ASSERT_EQ(report.residual, 1.0);
}


static void
test_zero_derivative(void)
{
    NA_SolverOption option = {
        .derivative_tolerance = 1e-12,
        .max_iterations = 100,
    };

    NA_RootReport report =
        NA_newton_raphson(
            f_zero_derivative,
            df_zero_derivative,
            1.0,
            option
        );

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_SMALL_DERIVATIVE);
    TF_TEST_ASSERT_EQ(report.root, 1.0);
    TF_TEST_ASSERT_EQ(report.residual, 2.0);
    TF_TEST_ASSERT_EQ(report.iterations, (NA_Size)0);
    TF_TEST_ASSERT_EQ(report.function_evaluations, (NA_Size)1);
    TF_TEST_ASSERT_EQ(report.derivative_evaluations, (NA_Size)1);
}


static void
test_max_iterations(void)
{
    NA_SolverOption option = {
        .abs_step_tolerance = 1e-20,
        .rel_step_tolerance = 1e-20,
        .residual_tolerance = 1e-20,
        .derivative_tolerance = 1e-20,
        .max_iterations = 1,
    };

    NA_RootReport report =
        NA_newton_raphson(
            f_max_iterations,
            df_max_iterations,
            0.0,
            option
        );

    TF_TEST_ASSERT_EQ(report.error, NA_ERROR_ROOT_MAX_ITERATIONS);
    TF_TEST_ASSERT_EQ(report.iterations, (NA_Size)1);
    TF_TEST_ASSERT_EQ(report.root, 1.0);
    TF_TEST_ASSERT_SCALAR_EQ(report.residual, 2.0);
}


int
main(void)
{
    TF_TEST_SUITE_BEGIN("Newton-Raphson");

    TF_RUN_TEST(test_converges_to_root);
    TF_RUN_TEST(test_residual_convergence_at_initial_value);
    TF_RUN_TEST(test_step_convergence);
    TF_RUN_TEST(test_default_options);

    TF_RUN_TEST(test_invalid_function_pointer);
    TF_RUN_TEST(test_invalid_derivative_function_pointer);
    TF_RUN_TEST(test_nonfinite_initial_value);
    TF_RUN_TEST(test_nonfinite_function_evaluation);
    TF_RUN_TEST(test_nonfinite_derivative_evaluation);
    TF_RUN_TEST(test_zero_derivative);
    TF_RUN_TEST(test_max_iterations);

    TF_TEST_SUITE_END();
}
