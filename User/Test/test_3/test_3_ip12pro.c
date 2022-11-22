#include "test_3.h"

const test_module_id_ut ip12pro_id =
{
  .device = IPHONE,
  .model_h = IPHONE_12_PRO >> 8,
  .model_l = IPHONE_12_PRO,
  .number = 0,
};

extern int ip12mini_test_start(test_module_st* module, const char* title);

const test_st test_ip12pro = 
{
  &ip12pro_id, ip12mini_test_start, test_3_stop, test_3_is_running, "XP 12 PRO"
};
