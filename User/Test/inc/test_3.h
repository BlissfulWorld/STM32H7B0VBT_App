#ifndef _TEST_3_H
#define _TEST_3_H

#include "test.h"

typedef struct test_3_config_s
{
  common_config_info_st info;
  power_test_st power[12];
  muc_vdd_io_power_test_st mvi;
}test_3_config_st;

typedef struct test_3_s
{
  test_3_config_st* config;
  uint8_t   (*stage_get)                 (void);
  void      (*init)                      (void);
  void      (*start)                     (void);
  uint8_t   (*prox_get)                  (void);
  uint8_t   (*prox_is_rdy)               (void);
  uint8_t   (*als_get)                   (void);
  uint8_t   (*als_is_rdy)                (void);
  uint8_t   (*mic_get)                   (void);
  int       (*mic_record_start)          (void);
  int       (*mic_record_stop)           (void);
  int       (*mic_record_play)           (void);
  uint8_t   (*mic_record_get_progress)   (void);
  double    (*ma_get)                    (uint8_t idx);
  double    (*mv_get)                    (uint8_t idx);
  uint8_t   (*ma_is_update)              (uint8_t idx);
  uint8_t   (*codec_aout_to_hac_test_res)(void);
  int       (*spkramp_play)              (void);
  uint8_t   (*spkramp_test_res)          (void);
}test_3_st;

typedef struct test_3_interface_s
{
  const uint8_t* gnd_pin_list;

  uint8_t codec_aout_to_hac_conn_p_pin;
  uint8_t codec_aout_to_hac_conn_n_pin;

  uint8_t coil_to_spkramp_top_vsense_p_pin;
  uint8_t coil_to_spkramp_top_vsense_n_pin;

  uint8_t prox_inti_pin;
  uint8_t als_inti_pin;

  int (*prox_i2c_write_read)(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);
  int (*prox_i2c_init)(void);
  int (*prox_i2c_fix)(void);

  int (*als_i2c_write_read)(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);
  int (*als_i2c_init)(void);
  int (*als_i2c_fix)(void);
}test_3_interface_st;

extern const test_module_id_ut ip12mini_id;
extern const test_module_id_ut ip12_id;
extern const test_module_id_ut ip12pro_id;
extern const test_module_id_ut ip12promax_id;

int ip12mini_test_start(test_module_st* module, const char* title);
#define ip12_test_start       ip12mini_test_start
#define ip12pro_test_start    ip12mini_test_start
#define ip12promax_test_start ip12mini_test_start

int test_3_start(test_module_st* module, const test_3_interface_st* intf, const char* title);

#endif
