#include "pca9535.h"
#include "hw_i2c.h"

int pca9535_config_port(uint32_t chip, uint16_t config)
{
  uint8_t id;
  uint8_t buffer[3];

  if(chip == 0)
  {
    id = 0x4C;
  }
  else
  {
    id = 0x4E;
  }
  buffer[0] = 0x06;
  buffer[1] = config;
  buffer[2] = config >> 8;
  return hw_i2c_write_read(hw_i2c2, id, buffer, 3, 0, 0, 30);
}

int pca9535_set_output(uint32_t chip, uint16_t state)
{
  uint8_t id;
  uint8_t buffer[3];

  if(chip == 0)
  {
    id = 0x4C;
  }
  else
  {
    id = 0x4E;
  }

  buffer[0] = 0x02;
  buffer[1] = state;
  buffer[2] = state >> 8;
  return hw_i2c_write_read(hw_i2c2, id, buffer, 3, 0, 0, 30);
}

int pca9535_get_output(uint32_t chip, uint16_t* output)
{
  uint8_t id;
  uint8_t buffer[3];

  if(chip == 0)
  {
    id = 0x4C;
  }
  else
  {
    id = 0x4E;
  }

  buffer[0] = 0x02;
  buffer[1] = 0;
  buffer[2] = 0;

  if(hw_i2c_write_read(hw_i2c2, id, buffer, 1, &buffer[1], 2, 30) == 0)
  {
    *output = buffer[1] | (buffer[2] << 8);
    return 0;
  }
  else
  {
    return -1;
  }
}
