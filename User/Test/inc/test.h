#ifndef _TEST_H
#define _TEST_H

#include "stdint.h"
#include "hw_pin.h"
#include "test_board.h"

typedef struct power_test_s
{
  //const char **json_obj;
  uint8_t en;
  double vdd;
  double i_max;
  double i_min;
  double v_max;
  double v_min;
  uint32_t color_in;
  uint32_t color_out;
  char alarm[16];
}power_test_st;

typedef struct muc_vdd_io_power_test_s
{
  //const char **json_obj;
  double vdd;
  double i_max;
  double i_min;
  double v_max;
  double v_min;
  uint32_t color_in;
  uint32_t color_out;
  char alarm[16];
}muc_vdd_io_power_test_st;

extern const hw_pin_st test_pin[51];
extern test_module_st *module;

#define TEST_STAGE_WAIT (0)
#define TEST_STAGE_OS   (1)
#define TEST_STAGE_FUNC (2)

void test_init(void);
int test_start(test_module_st *module);
double test_ma_get(uint8_t idx);
double test_mv_get(uint8_t idx);
uint8_t test_ma_is_update(uint8_t idx);

#include "test_0.h"

#endif
