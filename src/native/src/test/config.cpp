/*!
 * \author David
 * \date 21-Jun-16.
 */

#include "config.h"
#include "../config/config_parser.h"
#include "test_utils.h"

static void test_output_config() {
    config_parser parser("config/config.json");
}

void config::run_all() {
    run_test(test_output_config, "test_output_config");
}