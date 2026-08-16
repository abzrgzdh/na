/// Usage:
///
///     // In exactly one source file:
///     #define NA_IMPLEMENTATION
///     #include "na.h"
///
///     // In all other source files:
///     #include "na.h"
///
/// Define NA_IMPLEMENTATION in exactly one translation unit.  Defining it in
/// more than one translation unit causes multiple definition errors, i.e., a
/// violation of One-Definition Rule (ODR).
///
/// Names beginning with NA__ are internal implementation details and should not
/// be used by applications.
///

#ifndef NA_H_
#define NA_H_

// ----------------------------------------------------------------------------------------------
//                                          <utils>
// ----------------------------------------------------------------------------------------------

//
// <utils.common_macros>
//

#ifndef NA_DEF                   // __reference__: nob.h by tsoding
///
/// Goes before declarations and definitions of the na functions. Useful to `#define NADEF static
/// inline` if your source code is a single file and you want the compiler to remove unused
/// functions.
///
#define NA_DEF
#endif  // NADEF

#ifndef NA_ASSERT               // __reference__: nob.h by tsoding
#include <assert.h>
#define NA_ASSERT assert
#endif  // NA_ASSERT

#ifndef NA_REALLOC              // __reference__: nob.h by tsoding
#include <stdlib.h>
#define NA_REALLOC realloc
#endif  // NA_REALLOC

#ifndef NA_FREE                 // __reference__: nob.h by tsoding
#include <stdlib.h>
#define NA_FREE free
#endif  // NA_FREE


//
// <utils.common_headers>
//

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <time.h>
#include <tgmath.h>  // Includes both math.h and complex.h


//
// <utils.types>
//

typedef bool          NA_Bool;

typedef double        NA_Scalar;
#define NA_FMT_SCALAR "%g"

typedef int32_t       NA_Int;
#define NA_FMT_INT    "%d"

typedef int32_t       NA_Uint;
#define NA_FMT_UINT   "%u"

typedef size_t        NA_Size;
#define NA_FMT_SIZE   "%zu"

typedef char         *NA_String;
typedef char          NA_Char;
#define NA_FMT_CHAR   "%c"


//
// <utils.float>
//

/// A value representing: 0.0 / 0.0
#define NA_SCALAR_NAN NAN // or: strtod("NaN", NULL)

/// A value representing: 1.0 / 0.0
#define NA_SCALAR_INF INFINITY // or: strtod("Inf", NULL)

/// Value of epsilon associated with the chosen Scalar type.
#define NA_SCALAR_EPSILON DBL_EPSILON

/// Smallest positive subnormal value representable by NA_Scalar.
#define NA_SCALAR_TRUE_MIN DBL_TRUE_MIN


//
// <utils.math>
//

/// Returns -1, 0, or 1, if the number is negative, zero, or positive,
/// respectively.
///
/// Check out this discussion on a portable solution:
///     https://stackoverflow.com/q/1903954
NA_Int NA_Scalar_signum(NA_Scalar number);


//
// <utils.functions>
//

// TODO: How to make this generic: Function?
// typedef NA_Scalar* (*NA_Function)(Size input_count, NA_Scalar*, Size output_count, NA_Scalar*)?
typedef NA_Scalar (*NA_FuncR1R1)(NA_Scalar);

// This is mainly used in tests.
#define DEFINE_FUNCR1R1(NAME, INPUT_PARAM, RESULT)                             \
    [[maybe_unused]] static NA_Scalar                                          \
    NAME([[maybe_unused]] NA_Scalar INPUT_PARAM)                               \
    {                                                                          \
        return (RESULT);                                                       \
    }


// ------------------------------------------------------------------------------------------------
//                                              <roots>
// ------------------------------------------------------------------------------------------------

//
// <roots.error>
//

/// The general error enumeration.
///
/// Note: non-finite means either Nan or Inf
typedef enum {
    /// No error has occurred.
    NA_ERROR_ROOT_SUCCESS,

    /// The bracket range (either from start or end side) is
    /// non-finite. (specific to bracketing method)
    NA_ERROR_ROOT_NONFINITE_BRACKET_RANGE,
    /// The initial guess is non-finite.
    NA_ERROR_ROOT_NONFINITE_INITIAL_VALUE,

    /// The most recent iterate value (root) is non-finite. For instance, an Inf
    /// value could indicate Newton's method diverged or overflowed.
    NA_ERROR_ROOT_NONFINITE_ITERATE,

    /// The function evaluation has resulted to a non-finite value. This could
    /// indicate a domain problem, e.g., \( f(x) = \log(x), \quad x < 0 \).
    NA_ERROR_ROOT_NONFINITE_FUNCTION_EVALUATION,

    /// The derivative function evaluation has resulted to a non-finite value.
    NA_ERROR_ROOT_NONFINITE_DERIVATIVE_EVALUATION,

    /// The derivative function at the current approximate is smaller than a
    /// tolerance.
    NA_ERROR_ROOT_SMALL_DERIVATIVE,

    /// Maximum number of iterations has reached.
    NA_ERROR_ROOT_MAX_ITERATIONS,

    /// The step size, Δx, became zero (or below NA_SCALAR_TRUE_MIN).
    NA_ERROR_ROOT_STAGNATION,

    /// There is no root inside the bracket (specific to bracketing methods).
    /// i.e., signum(bracket_start) == signnum(bracket_end)
    NA_ERROR_ROOT_INVALID_BRACKET,

    /// Invalid solver option.
    NA_ERROR_ROOT_INVALID_SOLVER_OPTION,

    /// Invalid input function pointer (nullptr).
    NA_ERROR_ROOT_INVALID_FUNC_POINTER,

    /// A sentinel value indicating the number of valid errors. This is useful
    /// for operations that require bound-checking.
    NA_ERROR_ROOT_COUNT,
} NA_RootError;

/// Returns the error message associated with a particular error.
NA_DEF NA_Char const *
NA_RootError_get_message(NA_RootError error);


//
// <roots.report>
//

/// Complete result of a root-finding calculation.
///
/// Fields that are not applicable to a particular method remain zero.
///
/// On failure, the fields contain the most recent values available at the
/// point where the solver terminated.
typedef struct {
    /// Reason why the solver terminated.
    NA_RootError error;

    /// Most recent root approximation.
    NA_Scalar    root;

    /// Absolute residual |f(root)| when available.
    NA_Scalar    residual;

    /// Most recent Newton/update step. Used by open methods.
    NA_Scalar    step;

    /// Most recent bracket size. Used by bracketing methods.
    NA_Scalar    bracket;

    /// Number of completed iterations, i.e., the number of completed updates to
    /// the approximation. For example:
    ///
    ///     initial guess
    ///         |
    ///         +-- f(x0) already sufficiently small -> return
    ///
    ///     iterations = 0
    ///
    /// and:
    ///
    ///     x0 -> x1
    ///            |
    ///            +-- converged
    ///
    ///     iterations = 1
    NA_Size      iterations;

    /// Number of function evaluations.
    NA_Size      function_evaluations;

    /// Number of derivative evaluations.
    NA_Size      derivative_evaluations;
} NA_RootReport;


//
// <roots.solver_options>
//

/// Solver options
///
/// - Fields not applicable to the solver are initialized to zero.
/// - A zero-valued option requests the library default; exact zero tolerance is
///   not supported, nor zero maximum number of iterations.
///
/// - Solver options cannot be negative. If they are zero, they get
///   default-initialized. The default values are those specified by
///   NA_SOLVEROPTION_DEFAULT_X macros.
///
/// QUESTION: does the following two require relative tolerance?
/// QUESTION: when do we need both absolute and relative tolerance?
typedef struct {
    //// Absolute and relative tolerances for the step size. (specific to open methods)
    NA_Scalar abs_step_tolerance;
    NA_Scalar rel_step_tolerance;

    //// Absolute and relative tolerances for the bracket size. (specific to bracketing methods)
    NA_Scalar abs_bracket_tolerance;
    NA_Scalar rel_bracket_tolerance;

    /// How close to zero the residual is
    NA_Scalar residual_tolerance;

    /// How close the derivative of the function at current approximate is
    NA_Scalar derivative_tolerance;

    /// Maximum number of iterations
    NA_Size   max_iterations;
} NA_SolverOption;

/// The default value for absolute and relative tolerances of the step size in
/// open root-finding methods, which update the value of root with a step.
#define NA_SOLVEROPTION_DEFAULT_ABS_STEP_TOLERANCE 1e-10
#define NA_SOLVEROPTION_DEFAULT_REL_STEP_TOLERANCE 1e-12

/// The default value for absolute and relative tolerances of the bracket size
/// in bracketing root-finding methods, e.g., bisection method, which update the
/// value of root with a bracket.
#define NA_SOLVEROPTION_DEFAULT_ABS_BRACKET_TOLERANCE 1e-10
#define NA_SOLVEROPTION_DEFAULT_REL_BRACKET_TOLERANCE 1e-12

/// The default value for a tolerance below which a number can be considered to
/// be equal to zero. It is used here to compare residuals and derivatives to
/// zero (e.g., Newton-Raphson method).
#define NA_SOLVEROPTION_DEFAULT_ZERO_TOLERANCE 1e-17

/// The default maximum number of iterations a root finding algorithm can
/// perform before we declare failure at convergence.
#define NA_SOLVEROPTION_DEFAULT_MAX_ITERATIONS 1000

/// Check the validity of solver options. Specifically, this checks whether all
/// floating-point options are finite and non-negative
NA_DEF bool
NA__SolverOption_is_valid(NA_SolverOption option);

/// Set default options if the user has not provided them. This happens when
/// during initialization (e.g., aggregate initialization) some of the fields
/// are initialized and not the rest, which C99+ initialize them to the default
/// value of their type.
NA_DEF void
NA__SolverOption_set_defaults(NA_SolverOption *option);


//
// <roots.newton_raphson>
//

/// Find a root of a scalar function using the Newton-Raphson method.
///
/// The Newton-Raphson method iteratively improves an estimate of a root of f(x) using the derivative
/// f'(x). Starting from an initial estimate x_n, the next estimate is computed as
///
///     x_(n+1) = x_n - f(x_n) / f'(x_n)
///
/// The method terminates successfully when either the function residual or the change in the
/// solution satisfies the configured convergence tolerances.
///
/// Unlike bracketing methods such as bisection, Newton-Raphson does not maintain an interval
/// containing a root. Consequently, convergence is not guaranteed for an arbitrary initial
/// value. The method generally converges rapidly when the initial estimate is sufficiently close to
/// a simple root and the derivative is well behaved near that root.
///
/// The derivative must be supplied explicitly by the caller. The derivative is evaluated at each
/// Newton iteration and must remain finite and sufficiently far from zero for the Newton update to
/// be well defined.
///
/// Parameters:
///
///     func      Function whose root is to be found.
///     dfunc     Derivative of `func`.
///     init_val  Initial estimate of the root.
///     option    Solver options controlling convergence tolerances and the
///               maximum number of iterations.
///
/// Returns:
///
///     A NA_RootReport containing the computed root, residual, iteration count, function evaluation
///     count, and an error code describing the outcome.
///
/// Errors:
///
///     NA_ERROR_ROOT_INVALID_FUNC_POINTER
///         `func` or `dfunc` is NULL.
///
///     NA_ERROR_ROOT_NONFINITE_INITIAL_VALUE
///         `init_val` is not finite.
///
///     NA_ERROR_ROOT_INVALID_SOLVER_OPTION
///         The supplied solver options are invalid.
///
///     NA_ERROR_ROOT_NONFINITE_FUNCTION_EVALUATION
///         `func` or `dfunc` returned a non-finite value during iteration.
///
///     NA_ERROR_ROOT_ZERO_DERIVATIVE
///         The derivative is zero at the current estimate and the Newton
///         update cannot be computed.
///
///     NA_ERROR_ROOT_SUCCESS
///         A root was found within the requested tolerances.
///
///     NA_ERROR_ROOT_MAX_ITERATIONS
///         The maximum number of iterations was reached before convergence.
///
/// Notes:
///
///     Newton-Raphson has quadratic convergence near a simple root when the function is sufficiently
///     smooth and the initial estimate is sufficiently close to the root. However, it may diverge,
///     cycle, or converge to a different root when started from an unsuitable initial value.
///
///     For problems where reliable convergence is more important than convergence speed, a
///     bracketing method such as NA_bisection() may be preferable.
///
NA_DEF NA_RootReport
NA_newton_raphson(
    NA_FuncR1R1     func,
    NA_FuncR1R1     dfunc,
    NA_Scalar       init_val,
    NA_SolverOption option
);


// ------------------
// <roots.bisection>
// ------------------

/// Find a root of a scalar function using the bisection method.
///
/// The bisection method is a bracketing root-finding algorithm based on the intermediate value
/// theorem. Given a continuous function f and an interval [a, b] for which f(a) and f(b) have
/// opposite signs, the method repeatedly bisects the interval and retains the half that continues to
/// bracket a root.
///
/// The initial bracket must satisfy:
///
///     f(a) * f(b) < 0
///
/// or, equivalently, f(a) and f(b) must have opposite signs. If either endpoint is already a root,
/// that endpoint is returned immediately.
///
/// At each iteration, the midpoint is computed as
///
///     m = a / 2 + b / 2
///
/// rather than `(a + b) / 2` to reduce the possibility of overflow when `a` and `b` have large
/// magnitudes.
///
/// The iteration terminates successfully when either:
///
///  1. The midpoint residual satisfies
///
///         |f(m)| < residual_tolerance
///
///     or
///
///  2. The bracket width satisfies
///
///         |b - a| <
///             abs_bracket_tolerance
///             + rel_bracket_tolerance * |m|
///
/// The method is guaranteed to converge to a root for a continuous function with a valid initial
/// bracket, subject to floating-point limitations and the configured maximum number of iterations.
///
/// Parameters:
///
///     func          Function whose root is to be found.
///     bracket_start Lower endpoint of the initial root bracket.
///     bracket_end   Upper endpoint of the initial root bracket.
///     option        Solver options controlling convergence and iteration
///                   limits.
///
/// Returns:
///
///     A NA_RootReport containing the computed root, residual, final bracket size, iteration count,
///     function evaluation count, and an error code describing the outcome.
///
/// Errors:
///
///     NA_ERROR_ROOT_INVALID_FUNC_POINTER
///         `func` is NULL.
///
///     NA_ERROR_ROOT_NONFINITE_BRACKET_RANGE
///         Either bracket endpoint is not finite.
///
///     NA_ERROR_ROOT_INVALID_SOLVER_OPTION
///         The supplied solver options are invalid.
///
///     NA_ERROR_ROOT_NONFINITE_FUNCTION_EVALUATION
///         The function returned a non-finite value at an endpoint or midpoint.
///
///     NA_ERROR_ROOT_INVALID_BRACKET
///         The initial endpoints do not bracket a root.
///
///     NA_ERROR_ROOT_SUCCESS
///         A root was found within the requested tolerances.
///
///     NA_ERROR_ROOT_MAX_ITERATIONS
///         The maximum number of iterations was reached before convergence.
///
/// Notes:
///
///     Bisection is generally slower than open root-finding methods such as Newton-Raphson, but it
///     has the important advantage of preserving a valid root bracket at every iteration and
///     therefore provides robust convergence when the function is continuous on the initial
///     interval.
///
NA_RootReport
NA_bisection(
    NA_FuncR1R1     func,
    NA_Scalar       bracket_start,
    NA_Scalar       bracket_end,
    NA_SolverOption option
);

#endif  // NA_H_


#ifdef NA_IMPLEMENTATION

//
// <impl.utils.math>
//

NA_DEF NA_Int
NA_Scalar_signum(NA_Scalar number)
{
    if (number == 0)
        return 0;

    return (NA_Int)copysign(1.0, number);
}


//
// <impl.root.error>
//

/// Error messages (constant array of string) associated one-to-one with the
/// error enumeration above.
static NA_Char const *const NA__RootError_messages[NA_ERROR_ROOT_COUNT] = {
    // NA_ERROR_ROOT_SUCCESS
    "No error has occurred. This means convergence.",

    // NA_ERROR_ROOT_NONFINITE_BRACKET_RANGE
    "The bracket range (either from start or end side) is non-finite.",

    // NA_ERROR_ROOT_NONFINITE_INITIAL_VALUE
    "The initial guess is non-finite.",

    // NA_ERROR_ROOT_NONFINITE_ITERATE
    "The most recent iterate value (root) is non-finite.",

    // NA_ERROR_ROOT_NONFINITE_FUNCTION_EVALUATION
    "The function evaluation has resulted to a non-finite value.",

    // NA_ERROR_ROOT_NONFINITE_DERIVATIVE_EVALUATION
    "The derivative function evaluation has resulted to a non-finite value.",

    // NA_ERROR_ROOT_SMALL_DERIVATIVE
    "The derivative function at the current approximate is smaller than a tolerance.",

    // NA_ERROR_ROOT_MAX_ITERATIONS
    "Maximum number of iterations has reached.",

    // NA_ERROR_ROOT_STAGNATION
    "The step size, Δx, became zero (or below NA_SCALAR_TRUE_MIN).",

    // NA_ERROR_ROOT_INVALID_BRACKET
    "There is no root inside the bracket (specific to bracketing methods).",

    // NA_ERROR_ROOT_INVALID_SOLVER_OPTION
    "Invalid solver option(s).",

    // NA_ERROR_ROOT_INVALID_FUNC_POINTER
    "Invalid input function pointer (nullptr).",

    // NA_ERROR_ROOT_COUNT
};

NA_DEF NA_Char const *
NA_RootError_get_message(NA_RootError error)
{
    if (error < NA_ERROR_ROOT_SUCCESS || error >= NA_ERROR_ROOT_COUNT)
        return "Unknown error.";
    return NA__RootError_messages[error];
}


//
// <impl.root.solver_option>
//

NA_DEF NA_Bool
NA__SolverOption_is_valid(NA_SolverOption option)
{
    return (
        isfinite(option.abs_step_tolerance)    && option.abs_step_tolerance    >= 0.0
     && isfinite(option.rel_step_tolerance)    && option.rel_step_tolerance    >= 0.0
     && isfinite(option.abs_bracket_tolerance) && option.abs_bracket_tolerance >= 0.0
     && isfinite(option.rel_bracket_tolerance) && option.rel_bracket_tolerance >= 0.0
     && isfinite(option.residual_tolerance)    && option.residual_tolerance    >= 0.0
     && isfinite(option.derivative_tolerance)  && option.derivative_tolerance  >= 0.0
        // option.max_iterations >= 0 <- always true
    );
}

NA_DEF void
NA__SolverOption_set_defaults(NA_SolverOption *option)
{
    if (option->abs_step_tolerance == 0.0)
        option->abs_step_tolerance = NA_SOLVEROPTION_DEFAULT_ABS_STEP_TOLERANCE;

    if (option->rel_step_tolerance == 0.0)
        option->rel_step_tolerance = NA_SOLVEROPTION_DEFAULT_REL_STEP_TOLERANCE;

    if (option->abs_bracket_tolerance == 0.0)
        option->abs_bracket_tolerance = NA_SOLVEROPTION_DEFAULT_ABS_BRACKET_TOLERANCE;

    if (option->rel_bracket_tolerance == 0.0)
        option->rel_bracket_tolerance = NA_SOLVEROPTION_DEFAULT_REL_BRACKET_TOLERANCE;

    if (option->residual_tolerance == 0.0)
        option->residual_tolerance = NA_SOLVEROPTION_DEFAULT_ZERO_TOLERANCE;

    if (option->derivative_tolerance == 0.0)
        option->derivative_tolerance = NA_SOLVEROPTION_DEFAULT_ZERO_TOLERANCE;

    if (option->max_iterations == 0)
        option->max_iterations = NA_SOLVEROPTION_DEFAULT_MAX_ITERATIONS;
}


//
// <impl.roots.newton_raphson>
//

NA_DEF NA_RootReport
NA_newton_raphson(
    NA_FuncR1R1 func,
    NA_FuncR1R1 dfunc,
    NA_Scalar init_val,
    NA_SolverOption option
)
{
    // Check input values
    if (!func || !dfunc)
        return (NA_RootReport){.error = NA_ERROR_ROOT_INVALID_FUNC_POINTER};
    if (!isfinite(init_val))
        return (NA_RootReport){.error = NA_ERROR_ROOT_NONFINITE_INITIAL_VALUE};
    if (!NA__SolverOption_is_valid(option))
        return (NA_RootReport){.error = NA_ERROR_ROOT_INVALID_SOLVER_OPTION};

    // Set default options if the user has not provided them.
    NA__SolverOption_set_defaults(&option);

    NA_Scalar x                      = init_val;
    NA_Scalar x_new                  = init_val;
    NA_Scalar step                   = {};
    NA_Scalar fx                     = {};
    NA_Scalar dfx                    = {};

    NA_Size   function_evaluations   = {};
    NA_Size   derivative_evaluations = {};

    for (NA_Size i = 0; i < option.max_iterations; i++) {
        // Check function evaluation at current approximate
        fx = func(x);
        function_evaluations++;

        NA_Bool function_nonfinite = !isfinite(fx);
        if (function_nonfinite) {
            return (NA_RootReport){
                .error                  = NA_ERROR_ROOT_NONFINITE_FUNCTION_EVALUATION,
                .root                   = x,
                .residual               = fx,
                .step                   = step,
                .iterations             = i,
                .function_evaluations   = function_evaluations,
                .derivative_evaluations = derivative_evaluations};
        }

        // Convergence check for residual
        NA_Bool residual_converged = fabs(fx) < option.residual_tolerance;
        if (residual_converged)
            return (NA_RootReport){
                .error                  = NA_ERROR_ROOT_SUCCESS,
                .root                   = x_new,
                .residual               = fabs(fx),
                .step                   = step,
                .iterations             = i,
                .function_evaluations   = function_evaluations,
                .derivative_evaluations = derivative_evaluations,
            };

        // Check derivative at current approximate
        dfx = dfunc(x);
        derivative_evaluations++;

        NA_Bool derivative_nonfinite = !isfinite(dfx);
        if (derivative_nonfinite)
            return (NA_RootReport){
                .error                  = NA_ERROR_ROOT_NONFINITE_DERIVATIVE_EVALUATION,
                .root                   = x,
                .residual               = fabs(fx),
                .step                   = step,
                .iterations             = i,
                .function_evaluations   = function_evaluations,
                .derivative_evaluations = derivative_evaluations,
            };

        NA_Bool derivative_small = fabs(dfx) < option.derivative_tolerance;
        if (derivative_small)
            return (NA_RootReport){
                .error                  = NA_ERROR_ROOT_SMALL_DERIVATIVE,
                .root                   = x,
                .residual               = fabs(fx),
                .step                   = step,
                .iterations             = i,
                .function_evaluations   = function_evaluations,
                .derivative_evaluations = derivative_evaluations,
            };

        // It is numerically more stable to calculate step per below, rather
        // than (x_new - x). This is because if, say, `x = 1e20` is a large
        // number, and `step = 1e-10` is tiny enough that `x + step == x` in
        // floating-point arithmetic, then `x_new - x` will be zero (the
        // iteration is effectively stuck) even though the calculated Newton
        // correction was not zero.
        step = -fx / dfx;

        // Newton update/correction
        x_new = x + step;

        // Check for stagnation
        NA_Bool stagnated = fabs(step) < NA_SCALAR_TRUE_MIN || x_new == x;
        if (stagnated)
            return (NA_RootReport){
                .error                  = NA_ERROR_ROOT_STAGNATION,
                .root                   = x_new,
                .residual               = fabs(fx),
                .step                   = step,
                .iterations             = i + 1,
                .function_evaluations   = function_evaluations,
                .derivative_evaluations = derivative_evaluations,
            };

        // Check if new estimate is finite (fx/dfx could be non-finite)
        NA_Bool iterate_nonfinite = !isfinite(x_new);
        if (iterate_nonfinite) {
            return (NA_RootReport){
                .error                  = NA_ERROR_ROOT_NONFINITE_ITERATE,
                .root                   = x_new,
                .residual               = x_new, // Use the same value of x_new for residual
                .step                   = x_new, // Use the same value of x_new for step
                .iterations             = i + 1,
                .function_evaluations   = function_evaluations,
                .derivative_evaluations = derivative_evaluations,
            };
        }

        // Convergence check for step size
        NA_Bool step_converged
            = fabs(step) <= option.abs_step_tolerance + option.rel_step_tolerance * fabs(x_new);
        if (step_converged) {
            fx = func(x_new);
            ++function_evaluations;

            if (!isfinite(fx))
                return (NA_RootReport){
                    .error                  = NA_ERROR_ROOT_NONFINITE_FUNCTION_EVALUATION,
                    .root                   = x_new,
                    .residual               = fx,
                    .step                   = step,
                    .iterations             = i + 1,
                    .function_evaluations   = function_evaluations,
                    .derivative_evaluations = derivative_evaluations,
                };

            return (NA_RootReport){
                .error                  = NA_ERROR_ROOT_SUCCESS,
                .root                   = x_new,
                .residual               = fabs(x_new),
                .step                   = step,
                .iterations             = i + 1,
                .function_evaluations   = function_evaluations,
                .derivative_evaluations = derivative_evaluations,
            };
        }
        x = x_new;
    }

    // Max number of iterations reached
    return (NA_RootReport){
        .error                  = NA_ERROR_ROOT_MAX_ITERATIONS,
        .root                   = x_new,
        .residual               = fabs(fx),
        .step                   = step,
        .iterations             = option.max_iterations,
        .function_evaluations   = function_evaluations,
        .derivative_evaluations = derivative_evaluations,
    };
}


//
// <impl.roots.bisection>
//

NA_RootReport
NA_bisection(
    NA_FuncR1R1     func,
    NA_Scalar       bracket_start,
    NA_Scalar       bracket_end,
    NA_SolverOption option)
{
    // Check function pointer are valid.
    if (!func)
        return (NA_RootReport){.error = NA_ERROR_ROOT_INVALID_FUNC_POINTER};

    // Check bracket ends are finite.
    if (!isfinite(bracket_start) || !isfinite(bracket_end))
        return (NA_RootReport){.error = NA_ERROR_ROOT_NONFINITE_BRACKET_RANGE};

    // Check solver options are valid.
    if (!NA__SolverOption_is_valid(option))
        return (NA_RootReport){.error = NA_ERROR_ROOT_INVALID_SOLVER_OPTION};

    // Set default options if the user has not provided them.
    NA__SolverOption_set_defaults(&option);

    // Evaluate bracket endpoints.
    NA_Scalar a  = bracket_start;
    NA_Scalar b  = bracket_end;
    NA_Scalar fa = func(a);
    NA_Scalar fb = func(b);

    NA_Size function_evaluations  = 2;

    // Check if function is finite in either endpoint.
    NA_Bool func_nonfinite_endpoints = !isfinite(fa) || !isfinite(fb);
    if (func_nonfinite_endpoints) {
        return (NA_RootReport){
            .error = NA_ERROR_ROOT_NONFINITE_FUNCTION_EVALUATION,
            .function_evaluations = function_evaluations,
        };
    }

    // Check for an actual root at either endpoint.
    if (fa == 0.0) {
        return (NA_RootReport){
            .error                = NA_ERROR_ROOT_SUCCESS,
            .root                 = a,
            .residual             = 0.0,
            .bracket              = b - a,
            .iterations           = 0,
            .function_evaluations = function_evaluations
        };
    }

    if (fb == 0.0) {
        return (NA_RootReport){
            .error                = NA_ERROR_ROOT_SUCCESS,
            .root                 = b,
            .residual             = 0.0,
            .bracket              = b - a,
            .iterations           = 0,
            .function_evaluations = function_evaluations
        };
    }

    // Check if there is at least one root
    // (corollary to intermediate value theorem)
    if (NA_Scalar_signum(fa) == NA_Scalar_signum(fb)) {
        return (NA_RootReport){.error = NA_ERROR_ROOT_INVALID_BRACKET};
    }

    NA_Scalar bracket = b - a;  // Bracket size

    NA_Scalar fm = {};
    NA_Scalar m  = {};          // Bracket midpoint

    for (NA_Size i = 0; i < option.max_iterations; i++) {
        // Performing arithmetic in the following way prevents overflow,
        // compared to `(a+b)/2`.
        m = (b / 2.0) + (a / 2.0);

        // Check function evaluation at the bracket midpoint.
        fm = func(m);
        function_evaluations++;
        if (!isfinite(fm)) {
            return (NA_RootReport){
                .error                = NA_ERROR_ROOT_NONFINITE_FUNCTION_EVALUATION,
                .root                 = m,
                .residual             = fm,
                .bracket              = bracket,
                .iterations           = i + 1,
                .function_evaluations = function_evaluations
            };
        }

        // Update bracket
        if (NA_Scalar_signum(fm) == NA_Scalar_signum(fa)) {
            a = m;
            fa = fm;
        } else {
            b = m;
            fb = fm;
        }

        // Update bracket size before proceeding with reporting a success
        // result. Otherwise we would report the value of bracket before the
        // update to the midpoint.
        bracket = b - a;
        assert((bracket < 0.0) == false); // Bracket size should not be negative.

        // Convergence check for residual
        NA_Bool residual_converged = fabs(fm) < option.residual_tolerance;
        if (residual_converged)
            return (NA_RootReport){
                .error                = NA_ERROR_ROOT_SUCCESS,
                .root                 = m,
                .residual             = fabs(fm),
                .bracket              = bracket,
                .iterations           = i + 1,
                .function_evaluations = function_evaluations,
            };

        // If bracket ends are almost equal, then we're at the root; either end
        // could be an answer.
        NA_Bool bracket_converged =
            bracket < option.abs_bracket_tolerance + option.rel_bracket_tolerance * fabs(m);
        if (bracket_converged)
            return (NA_RootReport){
                .error                = NA_ERROR_ROOT_SUCCESS,
                .root                 = m,
                .residual             = fabs(fm),
                .bracket              = bracket,
                .iterations           = i + 1,
                .function_evaluations = function_evaluations,
            };

    }

    // Max number of iterations reached
    return (NA_RootReport){
        .error                = NA_ERROR_ROOT_MAX_ITERATIONS,
        .root                 = m,
        .residual             = fabs(fm),
        .bracket              = bracket,
        .iterations           = option.max_iterations,
        .function_evaluations = function_evaluations,
    };
}

#endif // NA_IMPLEMENTATION
