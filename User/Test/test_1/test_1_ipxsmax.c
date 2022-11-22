#include "test_1.h"

const test_module_id_ut ipxsmax_id =
{
  .device = IPHONE,
  .model_h = IPHONE_XS_MAX >> 8,
  .model_l = IPHONE_XS_MAX,
  .number = 0,
};

extern int ipxs_test_start(test_module_st* module, const char* title);

const test_st test_ipxsmax = 
{
  &ipxsmax_id, ipxs_test_start, test_1_stop, test_1_is_running, "XP XS MAX"
};
