#include "test_2.h"

const test_module_id_ut ip11promax_id =
{
  .device = IPHONE,
  .model_h = IPHONE_11_PRO_MAX >> 8,
  .model_l = IPHONE_11_PRO_MAX,
  .number = 0,
};

extern int ip11pro_test_start(test_module_st* module, const char* title);

const test_st test_ip11promax = 
{
  &ip11promax_id, ip11pro_test_start, test_2_stop, test_2_is_running, "XP 11 PRO MAX"
};
