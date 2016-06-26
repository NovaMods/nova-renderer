/*!
 * \author David
 * \date 21-Jun-16.
 */

#include "config.h"
#include "test_utils.h"

static void test_output_config() {
    //config parser("config/config.json");
}

void config_ns::run_all() {
    run_test(test_output_config, "test_output_config");
}