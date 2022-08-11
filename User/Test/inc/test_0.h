#ifndef _TEST_0_H
#define _TEST_0_H

#include "test.h"

typedef struct test_0_config_s
{
  common_config_info_st info;
  power_test_st power[8];
  muc_vdd_io_power_test_st mvi;
}test_0_config_st;

typedef struct test_0_s
{
  test_0_config_st* config;
  uint8_t(*stage_get)              (void);
  void      (*init)                   (void);
  void      (*start)                  (void);
  uint8_t(*prox_get)               (void);
  uint8_t(*prox_is_rdy)            (void);
  uint8_t(*als_get)                (void);
  uint8_t(*als_is_rdy)             (void);
  uint8_t(*mic_get)                (void);
  int       (*mic_record_start)       (void);
  int       (*mic_record_stop)        (void);
  int       (*mic_record_play)        (void);
  uint8_t(*mic_record_get_progress)(void);
  uint8_t(*cam_is_rdy)             (void);
  double    (*ma_get)                 (uint8_t idx);
  double    (*mv_get)                 (uint8_t idx);
  uint8_t(*ma_is_update)           (uint8_t idx);
  uint8_t(*spk_pn_get)             (void);
}test_0_st;

typedef struct test_0_interface_s
{
  const uint8_t* gnd_pin_list;

  uint8_t spk_out_p_pin;
  uint8_t spk_out_n_pin;
  uint8_t spk_vsense_p_pin;
  uint8_t spk_vsense_n_pin;

  uint8_t prox_inti_pin;
  uint8_t als_inti_pin;

  int (*prox_i2c_write_read)(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);
  int (*prox_i2c_init)(void);
  int (*prox_i2c_fix)(void);

  int (*als_i2c_write_read)(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);
  int (*als_i2c_init)(void);
  int (*als_i2c_fix)(void);

  int (*cam_i2c_write_read)(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);
  int (*cam_i2c_init)(void);
  int (*cam_i2c_fix)(void);

}test_0_interface_st;

extern const test_module_id_ut ip7_id;
extern const test_module_id_ut ip7plus_id;
extern const test_module_id_ut ip8plus_id;

int ip7_test_start(test_module_st* module, const char* title);
int ip7plus_test_start(test_module_st* module, const char* title);
int ip8plus_test_start(test_module_st* module, const char* title);

int test_0_start(test_module_st* module, const test_0_interface_st* intf, const char* title);

#endif
