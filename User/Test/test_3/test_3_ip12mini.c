#include "test.h"
#include "hw_i2c.h"
#include "hw_sim_i2c.h"

#define PROX_I2C_ID 0xb0
#define ALS_I2C_ID 0x52


static int cam_i2c_init(void);
static int cam_i2c_write_read(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);

static int i2c_init(void);
static int i2c_fix(void);
static int i2c_write_read(uint8_t i2c_addr, const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);

static const uint8_t gnd_pin_list[];
static const uint8_t func_pin_list[22];

const test_module_id_ut ip12mini_id =
{
  .device = IPHONE,
  .model_h = IPHONE_12_MINI >> 8,
  .model_l = IPHONE_12_MINI,
  .number = 0,
};

static const test_3_interface_st intf =
{
  .gnd_pin_list = gnd_pin_list,
  .func_pin_list = func_pin_list,

  .codec_aout_to_hac_conn_p_pin = 44,
  .codec_aout_to_hac_conn_n_pin = 43,

  .coil_to_spkramp_top_vsense_p_pin = 10,
  .coil_to_spkramp_top_vsense_n_pin = 7,

  .prox_inti_pin = 38,
  .als_inti_pin = 35,

  .i2c_write_read = i2c_write_read,
  .i2c_init = i2c_init,
  .i2c_fix = i2c_fix,
};

static const test_3_config_st config_default =
{
#define VI_CONST_INIT(idx,enable) \
  /*.power[idx-1].json_obj = (const char *[]){json_obj_test,json_obj_power,json_obj_##idx,0},*/\
  .power[idx-1].i_max = 5000,\
  .power[idx-1].i_min = 0,\
  .power[idx-1].v_max = 5000,\
  .power[idx-1].v_min = 0,\
  .power[idx-1].color_in = 0xFFFFFF,\
  .power[idx-1].color_out = 0xFF0000,\
  .power[idx-1].en = enable

#define OST_CONST_INIT(idx,val) \
  .ost[idx-1].i_max = val + val * 0.05,\
  .ost[idx-1].i_min = val - val * 0.05,\
  .ost[idx-1].color_in = 0xFFFFFF,\
  .ost[idx-1].color_out = 0xFF0000

  VI_CONST_INIT(1,0),
  VI_CONST_INIT(2,1),
  VI_CONST_INIT(3,1),
  VI_CONST_INIT(4,1),
  VI_CONST_INIT(5,1),
  VI_CONST_INIT(6,1),
  VI_CONST_INIT(7,0),
  VI_CONST_INIT(8,0),
  VI_CONST_INIT(9,0),
  VI_CONST_INIT(10,0),
  VI_CONST_INIT(11,1),
  VI_CONST_INIT(12,0),

  .mvi.i_max = 5000,
  .mvi.i_min = 0,
  .mvi.v_max = 5000,
  .mvi.v_min = 0,
  .mvi.color_in = 0xFFFFFF,
  .mvi.color_out = 0xFF0000,

  OST_CONST_INIT(1, 198.0),
  OST_CONST_INIT(2, 199.0),
  OST_CONST_INIT(3, 197.0),
  OST_CONST_INIT(4, 198.0),
  OST_CONST_INIT(5, 198.0),
  OST_CONST_INIT(6, 198.0),
  OST_CONST_INIT(7, 197.0),
  OST_CONST_INIT(8, 198.0),
  OST_CONST_INIT(9, 198.0),
  OST_CONST_INIT(10, 198.0),
  OST_CONST_INIT(11, 198.0),
  OST_CONST_INIT(12, 198.0),
  OST_CONST_INIT(13, 198.0),
  OST_CONST_INIT(14, 198.0),
  OST_CONST_INIT(15, 203.0),
  OST_CONST_INIT(16, 202.0),
  OST_CONST_INIT(17, 198.0),
  OST_CONST_INIT(18, 197.0),
  OST_CONST_INIT(19, 198.0),
  OST_CONST_INIT(20, 197.0),
  OST_CONST_INIT(21, 198.0),

  .os_test_num = sizeof(func_pin_list) - 1,
};


static const uint8_t gnd_pin_list[] =
{
  11,21,22,23,24,0
};

static const uint8_t func_pin_list[22] =
{
  MP_PB9,MP_PA2,MP_PC7,MP_PD3,MP_PD6,MP_PD8,MP_PD13,MP_PD12,MP_PD5,MP_PE1,MP_PC13,
  MP_PB5,MP_PD2,MP_PA6,MP_PA7,MP_PA5,MP_PB8,MP_PD4,MP_PC8,MP_PC6,MP_PD11,
  0
};

static int i2c_init(void)
{
  return i2c_fix();
}

static int i2c_fix(void)
{
  return hw_i2c_fix(hw_i2c4, 1000);
}

static int i2c_write_read(uint8_t i2c_addr, const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout)
{
  return hw_i2c_write_read(hw_i2c4, i2c_addr, pw, wlen, pr, rlen, timeout);
}

int ip12mini_test_start(test_module_st* module, const char* title)
{
  return test_3_start(module, &intf, &config_default, title);
}
