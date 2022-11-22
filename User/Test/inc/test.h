#ifndef _TEST_H
#define _TEST_H

#include "stdint.h"
#include "hw_pin.h"
#include "test_board.h"
#include "config.h"
#include "user_settings.h"

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

typedef struct os_test_s
{
  //const char **json_obj;
  double i_max;
  double i_min;
  uint32_t color_in;
  uint32_t color_out;
  char alarm[16];
}os_test_st;

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

typedef struct test_s
{
  const test_module_id_ut* id;
  int (*start)(test_module_st* module, const char* title);
  void (*stop)(void);
  int (*is_running)(void);
  const char* title;
}test_st;

#define TEST_STAGE_WAIT (0)
#define TEST_STAGE_OS   (1)
#define TEST_STAGE_FUNC (2)

#define MP_PA6  (1)
#define MP_PA7  (2)
#define MP_PC3  (3)
#define MP_PB0  (4)
#define MP_PC4  (5)
#define MP_PC5  (6)
#define MP_PC6  (7)
#define MP_PC7  (8)
#define MP_PC8  (9)
#define MP_PA2  (10)
#define MP_PE15 (11)
#define MP_PE3  (12)
#define MP_PE7  (13)
#define MP_PE9  (14)
#define MP_PC1  (15)
#define MP_PD9  (16)
#define MP_PE0  (17)
#define MP_PD11 (18)
#define MP_PD12 (19)
#define MP_PD13 (20)
#define MP_PA3  (21)
#define MP_PA9  (22)
#define MP_PA10 (23)
#define MP_PB1  (24)

#define MP_PA8  (27)
#define MP_PE6  (28)
#define MP_PE5  (29)
#define MP_PE12 (30)
#define MP_PE4  (31)
#define MP_PD7  (32)
#define MP_PB4  (33)
#define MP_PA4  (34)
#define MP_PA5  (35)
#define MP_PB8  (36)
#define MP_PB7  (37)
#define MP_PE1  (38)
#define MP_PD10 (39)
#define MP_PD8  (40)
#define MP_PD6  (41)
#define MP_PD5  (42)
#define MP_PD4  (43)
#define MP_PD3  (44)
#define MP_PD2  (45)
#define MP_PD1  (46)
#define MP_PD0  (47)
#define MP_PC13 (48)
#define MP_PB9  (49)
#define MP_PB5  (50)

extern const hw_pin_st test_pin[51];
extern test_module_st* module;


void test_init(void);
int test_start(test_module_st* module);
int test_stop(test_module_st* module);

void test_current_start(void);
void test_current_stop(void);

double test_ma_get(uint8_t idx);
double test_mv_get(uint8_t idx);
uint8_t test_ma_is_update(uint8_t idx);

#include "os_test.h"

#endif
