#include "test.h"
#include "hw_i2c.h"
#include "hw_sim_i2c.h"

static int cam_i2c_init(void);
static int cam_i2c_write_read(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);

static int i2c_init(void);
static int i2c_fix(void);
static int i2c_write_read(uint8_t i2c_addr, const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);

static const uint8_t gnd_pin_list[];

const test_module_id_ut ip11_id =
{
  .device = IPHONE,
  .model_h = IPHONE_11 >> 8,
  .model_l = IPHONE_11,
  .number = 0,
};

static const test_2_interface_st intf =
{
  .gnd_pin_list = gnd_pin_list,

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


static const uint8_t gnd_pin_list[] =
{
  11,12,21,22,23,24,33,0
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

int ip11_test_start(test_module_st* module, const char* title)
{
  return test_2_start(module, &intf, title);
}
