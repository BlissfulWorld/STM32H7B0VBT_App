#include "test_0.h"
#include "hw_i2c.h"
#include "hw_sim_i2c.h"

#define PROX_I2C_ID 0xb0
#define ALS_I2C_ID 0x52


static int cam_i2c_init(void);
static int cam_i2c_write_read(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);

static int prox_i2c_init(void);
static int prox_i2c_fix(void);
static int prox_i2c_write_read(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);

static int als_i2c_init(void);
static int als_i2c_fix(void);
static int als_i2c_write_read(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);

static const uint8_t gnd_pin_list[];
static const uint8_t func_pin_list[];

const test_module_id_ut ip7plus_id =
{
  .device = IPHONE,
  .model_h = IPHONE_7P >> 8,
  .model_l = IPHONE_7P,
  .number = 0,
};

static const test_0_interface_st intf =
{
  .gnd_pin_list = gnd_pin_list,
  .func_pin_list = func_pin_list,

  .spk_out_p_pin = 8,
  .spk_out_n_pin = 9,
  .spk_vsense_p_pin = 10,
  .spk_vsense_n_pin = 7,

  .prox_inti_pin = 38,
  .als_inti_pin = 35,

  .cam_i2c_init = cam_i2c_init,
  .cam_i2c_fix = 0,
  .cam_i2c_write_read = cam_i2c_write_read,

  .prox_i2c_write_read = prox_i2c_write_read,
  .prox_i2c_init = prox_i2c_init,
  .prox_i2c_fix = prox_i2c_fix,

  .als_i2c_write_read = als_i2c_write_read,
  .als_i2c_init = als_i2c_init,
  .als_i2c_fix = als_i2c_fix,
};

static const test_0_config_st config_default =
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

  VI_CONST_INIT(1,1),
  VI_CONST_INIT(2,1),
  VI_CONST_INIT(3,1),
  VI_CONST_INIT(4,1),
  VI_CONST_INIT(5,1),
  VI_CONST_INIT(6,1),
  VI_CONST_INIT(7,0),
  VI_CONST_INIT(8,0),
  //VI_CONST_INIT(9,0),
  //VI_CONST_INIT(10,0),
  //VI_CONST_INIT(11,0),
  //VI_CONST_INIT(12,0),

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
  OST_CONST_INIT(22, 198.0),
  OST_CONST_INIT(23, 197.0),
  OST_CONST_INIT(24, 205.0),
  OST_CONST_INIT(25, 278.0),
  OST_CONST_INIT(26, 217.0),
  OST_CONST_INIT(27, 217.0),
  OST_CONST_INIT(28, 278.0),
  OST_CONST_INIT(29, 217.0),
  OST_CONST_INIT(30, 209.0),
  OST_CONST_INIT(31, 277.0),
  OST_CONST_INIT(32, 217.0),
  OST_CONST_INIT(33, 217.0),

  .os_test_num = 33,
};


static const uint8_t gnd_pin_list[] =
{
  11,12,21,22,23,24,0
};

static const uint8_t func_pin_list[] =
{
  MP_PB5,MP_PB9,MP_PB4,MP_PA8,MP_PE0,MP_PB8,MP_PE7,MP_PD0,MP_PC6,MP_PC8,MP_PD13,MP_PD12,MP_PE1,MP_PD2,MP_PA6,
  MP_PA7,MP_PC7,MP_PA2,MP_PC13,MP_PE9,MP_PB7,MP_PA5,MP_PD11,MP_PD1,MP_PA3,MP_PD9,MP_PC1,MP_PE15,MP_PB0,MP_PC3,MP_PE3,MP_PC5,MP_PC4,
  0
};

static int cam_i2c_init(void)
{
  return hw_sim_i2c_set_pin(hw_sim_i2c1, &test_pin[17], &test_pin[18], 100);
}

static int cam_i2c_write_read(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout)
{
  return hw_sim_i2c_write_read(hw_sim_i2c1, 0x20, pw, wlen, pr, rlen, timeout);
}

static int prox_i2c_init(void)
{
  return prox_i2c_fix();
}

static int prox_i2c_fix(void)
{
  return hw_i2c_fix(hw_i2c4, 1000);
}

static int prox_i2c_write_read(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout)
{
  return hw_i2c_write_read(hw_i2c4, PROX_I2C_ID, pw, wlen, pr, rlen, timeout);
}

static int als_i2c_init(void)
{
  return als_i2c_fix();
}

static int als_i2c_fix(void)
{
  return hw_i2c_fix(hw_i2c1, 1000);
}

static int als_i2c_write_read(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout)
{
  return hw_i2c_write_read(hw_i2c1, ALS_I2C_ID, pw, wlen, pr, rlen, timeout);
}

int ip7plus_test_start(test_module_st* module, const char* title)
{
  return test_0_start(module, &intf, &config_default, title);
}

const test_st test_ip7plus = 
{
  &ip7plus_id, ip7plus_test_start, test_0_stop, test_0_is_running, "XP 7 PLUS"
};
