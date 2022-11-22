#ifndef _test_2_H
#define _test_2_H

#include "test.h"

typedef struct test_2_config_s
{
  common_config_info_st info;
  power_test_st power[8];
  muc_vdd_io_power_test_st mvi;
  os_test_st ost[50];
  uint8_t os_test_num;
}test_2_config_st;

typedef struct test_2_s
{
  test_2_config_st* config;
  uint8_t       (*stage_get)                 (void);
  void          (*init)                      (void);
  void          (*start)                     (void);
  const float*  (*os_test_data_get)          (uint8_t *len);
  uint8_t       (*os_test_result_get)        (void);
  uint8_t       (*prox_get)                  (void);
  uint8_t       (*prox_is_rdy)               (void);
  uint8_t       (*als_get)                   (void);
  uint8_t       (*als_is_rdy)                (void);
  uint8_t       (*mic_get)                   (void);
  int           (*mic_record_start)          (void);
  int           (*mic_record_stop)           (void);
  int           (*mic_record_play)           (void);
  uint8_t       (*mic_record_get_progress)   (void);
  double        (*ma_get)                    (uint8_t idx);
  double        (*mv_get)                    (uint8_t idx);
  uint8_t       (*ma_is_update)              (uint8_t idx);
  uint8_t       (*codec_aout_to_hac_test_res)(void);
  int           (*spkramp_play)              (void);
  uint8_t       (*spkramp_test_res)          (void);
  uint8_t       (*yogi_is_rdy)               (void);
}test_2_st;

typedef struct test_2_interface_s
{
  const uint8_t* gnd_pin_list;
  const uint8_t* func_pin_list;

  uint8_t codec_aout_to_hac_conn_p_pin;
  uint8_t codec_aout_to_hac_conn_n_pin;

  uint8_t coil_to_spkramp_top_vsense_p_pin;
  uint8_t coil_to_spkramp_top_vsense_n_pin;

  uint8_t prox_inti_pin;
  uint8_t als_inti_pin;

  int (*i2c_write_read)(uint8_t i2c_addr, const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);
  int (*i2c_init)(void);
  int (*i2c_fix)(void);
}test_2_interface_st;

int test_2_start(test_module_st* module, const test_2_interface_st* intf, const test_2_config_st* default_config, const char* title);
void test_2_stop(void);
int test_2_is_running(void);

extern void ui_test_2_open(test_2_st* _test_, const char* _title_);
#endif
