#ifndef _TEST_4_H
#define _TEST_4_H

#include "test.h"

typedef struct test_4_config_s
{
  common_config_info_st info;
  power_test_st power[12];
  muc_vdd_io_power_test_st mvi;
  os_test_st ost[50];
  uint8_t os_test_num;
}test_4_config_st;

typedef struct test_4_io_s
{
  uint8_t num;
  uint8_t mcu_pin;
}test_4_io_st;

typedef struct test_4_s
{
  test_4_config_st* config;
  uint8_t       (*stage_get)                 (void);
  uint8_t       (*mic1_is_rdy)               (void);
  uint8_t       (*mic_get)                   (void);
  int           (*mic_record_start)          (void);
  int           (*mic_record_stop)           (void);
  int           (*mic_record_play)           (void);
  uint8_t       (*mic_record_get_progress)   (void);
  double        (*ma_get)                    (uint8_t idx);
  double        (*mv_get)                    (uint8_t idx);
  uint8_t       (*ma_is_update)              (uint8_t idx);
  int           (*io_test_error_get)         (void);
  uint32_t      (*io_short_circuit_count_get)(void);
  uint16_t*     (*io_short_circuit_data_get) (void);
  uint8_t*      (*io_open_circuit_data_get)  (void);
  uint8_t       (*io_open_circuit_count_get) (void);
}test_4_st;

extern void ui_test_4_open(test_4_st* _test_, const char* _title_);
#endif
