#include "test_0.h"

const test_module_id_ut ip8_id =
{
  .device = IPHONE,
  .model_h = IPHONE_8 >> 8,
  .model_l = IPHONE_8,
  .number = 0,
};

extern int ip8plus_test_start(test_module_st* module, const char* title);

const test_st test_ip8 =
{
  &ip8_id, ip8plus_test_start, test_0_stop, test_0_is_running, "XP 8"
};
