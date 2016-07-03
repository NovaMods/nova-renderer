/*!
 * \author David
 * \date 17-May-16.
 */

#include <easylogging++.h>

#include "test_utils.h"

void run_test(test_func test_function, std::string name) {
    LOG(INFO) << "Starting test " << name;
    try {
        test_function();
    } catch(std::exception & e) {
        LOG(ERROR) << "Exception with message " << e.what();
    }
}
