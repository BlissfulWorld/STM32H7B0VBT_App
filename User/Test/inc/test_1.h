#ifndef _TEST_1_H
#define _TEST_1_H

#include "test.h"

typedef struct test_1_config_s
{
  common_config_info_st info;
  power_test_st power[8];
  muc_vdd_io_power_test_st mvi;
}test_1_config_st;

typedef struct test_1_s
{
  test_1_config_st* config;
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
  uint8_t   (*yogi_is_rdy)               (void);
  uint8_t   (*hall_get)                  (void);
}test_1_st;

typedef struct test_1_interface_s
{
  const uint8_t* gnd_pin_list;

  uint8_t codec_aout_to_hac_conn_p_pin;
  uint8_t codec_aout_to_hac_conn_n_pin;

  uint8_t coil_to_spkramp_top_vsense_p_pin;
  uint8_t coil_to_spkramp_top_vsense_n_pin;

  uint8_t prox_inti_pin;
  uint8_t als_inti_pin;

  int (*i2c_write_read)(uint8_t i2c_addr, const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);
  int (*i2c_init)(void);
  int (*i2c_fix)(void);
}test_1_interface_st;

extern const test_module_id_ut ipx_id;
extern const test_module_id_ut ipxs_id;
extern const test_module_id_ut ipxsmax_id;
extern const test_module_id_ut ipxr_id;

int ipx_test_start(test_module_st* module, const char* title);
int ipxs_test_start(test_module_st* module, const char* title);
#define ipxsmax_test_start ipxs_test_start
#define ipxr_test_start    ipx_test_start

int test_1_start(test_module_st* module, const test_1_interface_st* intf, const char* title);

#endif
