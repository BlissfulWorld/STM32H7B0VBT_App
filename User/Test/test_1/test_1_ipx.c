#include "test.h"
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

const test_module_id_ut ipx_id =
{
  .device = IPHONE,
  .model_h = IPHONE_X >> 8,
  .model_l = IPHONE_X,
  .number = 0,
};

static const test_1_interface_st intf =
{
  .gnd_pin_list = gnd_pin_list,

  .codec_aout_to_hac_conn_p_pin = 44,
  .codec_aout_to_hac_conn_n_pin = 43,

  .prox_inti_pin = 38,
  .als_inti_pin = 35,

  .prox_i2c_write_read = prox_i2c_write_read,
  .prox_i2c_init = prox_i2c_init,
  .prox_i2c_fix = prox_i2c_fix,

  .als_i2c_write_read = als_i2c_write_read,
  .als_i2c_init = als_i2c_init,
  .als_i2c_fix = als_i2c_fix,
};


static const uint8_t gnd_pin_list[] =
{
  11,12,21,23,24,33,0
};

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
  return hw_i2c_fix(hw_i2c4, 1000);
}

static int als_i2c_write_read(const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout)
{
  return hw_i2c_write_read(hw_i2c4, ALS_I2C_ID, pw, wlen, pr, rlen, timeout);
}

int ipx_test_start(test_module_st* module, const char* title)
{
  return test_1_start(module, &intf, title);
}