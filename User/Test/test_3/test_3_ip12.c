#include "test_3.h"

const test_module_id_ut ip12_id =
{
  .device = IPHONE,
  .model_h = IPHONE_12 >> 8,
  .model_l = IPHONE_12,
  .number = 0,
};

extern int ip12mini_test_start(test_module_st* module, const char* title);

const test_st test_ip12 = 
{
  &ip12_id, ip12mini_test_start, test_3_stop, test_3_is_running, "XP 12"
};
