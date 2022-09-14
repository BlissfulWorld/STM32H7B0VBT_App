#ifndef _OS_TEST_H
#define _OS_TEST_H

#include "test.h"
#include "cJSON.h"

int os_test_get_volt(const uint8_t* gnd_list, const uint8_t* func_list, float* result);
int ostest_json_config_phase(cJSON* j_test, os_test_st* ost, uint8_t num);

#endif
