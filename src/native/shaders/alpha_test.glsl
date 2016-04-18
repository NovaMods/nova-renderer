#version 450

/*!
 * \brief Includes all the code you need to do alpha testing
 *
 * Uses shader subroutines to determine how to alpha test. There's one subroute type, 'alpha_test_performer(vec4, float)'
 * which has one function to perform an alpha test and one function to ignore tha alpha test. There's also another
 * subroutine type, 'alpha_test_func', which has one function for each of OpenGL's alpha test functions
 *
 * It's worth noting that the alpha test subroutine is given index 1 and the alpha test fucntion subroutine is given
 * index 2
 */

layout(index = 1) subroutine vec4 alpha_test_performer(in vec4 color, in float cutoff);
layout(index = 2) subroutine float alpha_test_func(in float alpha, in float comparison);

subroutine uniform alpha_test_performer do_alpha_test;
subroutine uniform alpha_test_func alpha_test_function;

uniform float alpha_test_cutoff;

/* Alpha tests */
layout(location = 1) subroutine(alpha_test_performer) vec4 alpha_test_enabled(in vec4 color) {
    return mix(vec4(0), color, alpha_test_function(color.a, alpha_test_cutoff));
}

layout(location = 2) subroutine(alpha_test_performer) vec4 alpha_test_disabled(in vec4 color, in float cutoff) {
    return color;
}

/* Alpha test functions */
layout(location = 3) subroutine(alpha_test_func) float alpha_test_never(in float alpha, in float comparison) {
    return 0.0f;
}

layout(location = 4) subroutine(alpha_test_func) float alpha_test_less(in float alpha, in float comparison) {
    return step(comparison, alpha);
}

layout(location = 5) subroutine(alpha_test_func) float alpha_test_equal(in float alpha, in float comparison) {
    return float(alpha == comparison);
}

layout(location = 6) subroutine(alpha_test_func) float alpha_test_greater(in float alpha, in float comparison) {
    return step(alpha, comparison);
}

layout(location = 7) subroutine(alpha_test_func) float alpha_test_less_equal(in float alpha, in float comparison) {
    return 1.0 - alpha_test_greater(alpha, comparison);
}

layout(location = 8) subroutine(alpha_test_func) float alpha_test_greater_equal(in float alpha, in float comparison) {
    return 1.0 - alpha_test_less(alpha, comparison);
}

layout(location = 9) subroutine(alpha_test_func) float alpha_test_not_equal(in float alpha, in float comparison) {
    return 1.0 - alpha_test_equal(alpha, comparison);
}

layout(location = 10) subroutine(alpha_test_func) float alpha_test_always(in float alpha, in float comparison) {
    return 1.0f;
}
