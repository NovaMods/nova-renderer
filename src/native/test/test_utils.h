/*!
 * \brief Contains simple utils to run tests
 *
 * Why am I doing this and not using a unit testing library? Google Test doesn't compile and I don't feel like
 * dealing with that
 *
 * \author David
 * \date 17-May-16.
 */

#ifndef RENDERER_TEST_UTILS_H
#define RENDERER_TEST_UTILS_H

#include <string>

typedef void (*test_func)();

void run_test(test_func test_function, std::string name);

#endif //RENDERER_TEST_UTILS_H
