#include "w25qxx.h"
#include "octospi.h"
#include "stdio.h"

#ifdef HAL_QSPI_TIMEOUT_DEFAULT_VALUE
#undef HAL_QSPI_TIMEOUT_DEFAULT_VALUE
#define HAL_QSPI_TIMEOUT_DEFAULT_VALUE (200)
#endif

#define W25QXX_DBG(...) do{}while(0)//printf(__VA_ARGS__)//

#define SPI_MODE_SINGLE 0
#define SPI_MODE_DUAL 1
#define SPI_MODE_QUAD 2

#define SPI_MODE SPI_MODE_QUAD

#define W25QXX_SECTOR_SIZE   (4*1024)

#define READ_STATUS_REGISTER_1  (0x05)
#define WRITE_STATUS_REGISTER_2 (0x31)
#define READ_STATUS_REGISTER_2  (0x35)
#define CHIP_ERASE              (0xC7)
#define SECTOR_ERASE            (0x20)
#define ENABLE_RESET            (0x66)
#define RESET_DEVICE            (0x99)

#define WRITE_ENABLE            (0x06)
#define FAST_READ_QUAD_OUTPUT   (0x6B)
#define QUAD_INPUT_PAGE_PROGRAM (0x32)

w25qxx_st* w25qxx = 0;
static w25qxx_st _w25qxx_;
static OSPI_RegularCmdTypeDef cmd;

static uint32_t w25qxx_device_id_get(void);
static int w25qxx_read(void* pdata, uint32_t address, uint32_t size);
static int w25qxx_write(void* pdata, uint32_t address, uint32_t size);
static int w25qxx_erase_sector(uint32_t sector);
static int w25qxx_erase_chip(void);

static uint32_t qspi_auto_polling_mem_ready(uint32_t timeout)
{
  OSPI_AutoPollingTypeDef cfg;

  cmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
  cmd.DataMode = HAL_OSPI_DATA_1_LINE;
  cmd.DummyCycles = 0;

  cmd.NbData = 1;
  cmd.Instruction = READ_STATUS_REGISTER_1;

  if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return -1;

  cfg.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;
  cfg.Interval = 0xA0;
  cfg.Mask = 0x01;
  cfg.Match = 0x00;
  cfg.MatchMode = HAL_OSPI_MATCH_MODE_AND;

  if(HAL_OSPI_AutoPolling(&hospi1, &cfg, timeout) != HAL_OK)
    return -2;

  return 0;
}

static int device_reset(void)
{
  cmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
  cmd.DataMode = HAL_OSPI_DATA_NONE;
  cmd.NbData = 0;
  cmd.DummyCycles = 0;

  cmd.Instruction = 0x66;

  if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return -1;

  return 0;
}

static int write_enable(void)
{
  OSPI_AutoPollingTypeDef cfg;

  cmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
  cmd.DataMode = HAL_OSPI_DATA_NONE;
  cmd.NbData = 0;
  cmd.DummyCycles = 0;
  cmd.Instruction = WRITE_ENABLE;

  if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return -1;

  cmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
  cmd.DataMode = HAL_OSPI_DATA_1_LINE;
  cmd.DummyCycles = 0;

  cmd.NbData = 1;
  cmd.Instruction = READ_STATUS_REGISTER_1;

  if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return -1;

  cfg.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;
  cfg.Interval = 0x10;
  cfg.Mask = 0x02;
  cfg.Match = 0x02;
  cfg.MatchMode = HAL_OSPI_MATCH_MODE_AND;

  if(HAL_OSPI_AutoPolling(&hospi1, &cfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return -3;

  return 0;
}

static uint32_t qspi_quad_enable(void)
{
  uint8_t value = 0;

  cmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
  cmd.DataMode = HAL_OSPI_DATA_1_LINE;
  cmd.DummyCycles = 0;

  cmd.NbData = 1;

  cmd.Instruction = READ_STATUS_REGISTER_2;
  if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return -1;

  if(HAL_OSPI_Receive(&hospi1, &value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return -2;

  if(value & 0x02) return 0;

  if(write_enable() != HAL_OK)
    return -3;

  cmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
  cmd.DataMode = HAL_OSPI_DATA_1_LINE;
  cmd.DummyCycles = 0;

  cmd.NbData = 1;

  cmd.Instruction = WRITE_STATUS_REGISTER_2;

  if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return -4;

  value |= 0x02;
  if(HAL_OSPI_Transmit(&hospi1, &value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return -5;

  return 0;
}


static int qspi_enter_4byte_mode(void)
{
  if(write_enable() != HAL_OK)
    return -1;

  cmd.Instruction = 0xB7;
  cmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
  cmd.DataMode = HAL_OSPI_DATA_NONE;
  cmd.NbData = 0;
  cmd.DummyCycles = 0;

  if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return -2;

  return 0;
}


static int w25qxx_read(void* pdata, uint32_t address, uint32_t size)
{
  uint32_t len;

  if(size == 0)    return 0;
  if(pdata == 0)   return -1;

#if SPI_MODE == SPI_MODE_DUAL
  cmd.Instruction = 0x3B;
  cmd.DataMode = HAL_OSPI_DATA_2_LINES;
#elif SPI_MODE == SPI_MODE_QUAD
  cmd.Instruction = 0x6B;
  cmd.DataMode = HAL_OSPI_DATA_4_LINES;
#else
  cmd.Instruction = 0x0B;
  cmd.DataMode = HAL_OSPI_DATA_1_LINE;
#endif

  cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
  cmd.DummyCycles = 8;

  for(uint32_t i = 0;i < size;i += len)
  {
    len = size - i;
    if(len > 1024) len = 1024;

    cmd.Address = address + i;
    cmd.NbData = len;

    if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
      return -2;

    if(HAL_OSPI_Receive(&hospi1, (uint8_t*)pdata + i, HAL_OSPI_TIMEOUT_DEFAULT_VALUE * (size / W25QXX_SECTOR_SIZE + 1)) != HAL_OK)
      return -3;
  }

  return 0;
}

static int w25qxx_write(void* pdata, uint32_t address, uint32_t size)
{
  uint32_t e_addr, s_addr, len;
  uint8_t* p = pdata;

  if(size == 0)    return 0;

  e_addr = address + size;
  for(s_addr = address;s_addr < e_addr;s_addr += len)
  {
    len = e_addr - s_addr;
    if(len > 256)
      len = 256;

    if(write_enable() != HAL_OK)
      return -1;

#if SPI_MODE == SPI_MODE_QUAD
    cmd.Instruction = 0x32;
    cmd.DataMode = HAL_OSPI_DATA_4_LINES;
#else
    cmd.Instruction = 0x02;
    cmd.DataMode = HAL_OSPI_DATA_1_LINE;
#endif

    cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
    cmd.DummyCycles = 0;
    cmd.Address = s_addr;
    cmd.NbData = len;

    if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
      return -2;

    if(HAL_OSPI_Transmit(&hospi1, p, HAL_OSPI_TIMEOUT_DEFAULT_VALUE * (size / W25QXX_SECTOR_SIZE + 1)) != HAL_OK)
      return -3;

    if(qspi_auto_polling_mem_ready(HAL_OSPI_TIMEOUT_DEFAULT_VALUE * (size / W25QXX_SECTOR_SIZE + 1)))
      return -4;

    p += len;
  }

  return 0;
}



/**
  * @brief 扇区擦除 每扇区4KByte
  * @param address: 地址 (对应扇区号sector=address/(4*1024))
  * @retval 0:成功 !0:失败
  */
static int w25qxx_erase_sector(uint32_t address)
{
  address &= 0xFFFFF000;

  if(write_enable() != HAL_OK)
    return -1;

  cmd.Instruction = 0x20;
  cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
  cmd.Address = address & 0xFFFFF000;
  cmd.DataMode = HAL_OSPI_DATA_NONE;
  cmd.DummyCycles = 0;
  cmd.NbData = 0;

  if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return -2;

  if(qspi_auto_polling_mem_ready(HAL_OSPI_TIMEOUT_DEFAULT_VALUE))
    return -3;

  return 0;
}

/**
  * @brief 芯片全擦除
  * @retval 0:成功 !0:失败
  */
static int w25qxx_erase_chip(void)
{
  if(write_enable() != HAL_OK)
    return -1;

  cmd.Instruction = 0xC7;
  cmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
  cmd.DataMode = HAL_OSPI_DATA_NONE;
  cmd.NbData = 0;
  cmd.DummyCycles = 0;

  if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return -2;

  if(qspi_auto_polling_mem_ready(HAL_OSPI_TIMEOUT_DEFAULT_VALUE))
    return -3;

  return 0;
}

static uint32_t w25qxx_device_id_get(void)
{
  uint8_t pdata[2];


  cmd.Instruction = 0x90;
  cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
  cmd.Address = 0x00;
  cmd.DataMode = HAL_OSPI_DATA_1_LINE;
  cmd.DummyCycles = 0;
  cmd.NbData = 2;

  if(HAL_OSPI_Command(&hospi1, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return 0;

  if(HAL_OSPI_Receive(&hospi1, pdata, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return 0;

  return (pdata[1] | (pdata[0] << 8));
}

/*static void w25qxx_test(void)
{
  #include "rng.h"
  #include "stdlib.h"
  #include "string.h"
  uint32_t rng;
  uint8_t *ptmp0,*ptmp1;

  W25QXX_DBG("w25qxx_test.\n");
  ptmp0 = malloc(w25qxx->sector_size * 2);
  if(ptmp0)
  {
    ptmp1 = ptmp0 + w25qxx->sector_size;
    for(uint32_t sector = 0;sector < w25qxx->sector_num;sector++)
    {
      for(uint32_t i = 0;i < w25qxx->sector_size;i++)
      {
        HAL_RNG_GenerateRandomNumber(&hrng,&rng);
        ptmp0[i] = rng;
      }
      w25qxx->sector_erase(sector * w25qxx->sector_size);
      w25qxx->write(ptmp0,sector * w25qxx->sector_size,w25qxx->sector_size);
      w25qxx->read(ptmp1,sector * w25qxx->sector_size,w25qxx->sector_size);
      if(memcmp(ptmp0,ptmp1,w25qxx->sector_size))
      {
        W25QXX_DBG("\nfail,sector = %u.\n",sector);
        break;
      }
      else
      {
        if(sector % 40 == 0) W25QXX_DBG("\n");
        W25QXX_DBG(".");
      }
    }
  }
  W25QXX_DBG("END.\n");

  while(1);
}*/

int w25qxx_init(void)
{
  uint16_t id;

  cmd.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
  cmd.FlashId = HAL_OSPI_FLASH_ID_1;
  cmd.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
  cmd.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
  cmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
  cmd.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
  cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  cmd.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
  cmd.DQSMode = HAL_OSPI_DQS_DISABLE;
  cmd.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

  w25qxx = 0;

  W25QXX_DBG("w25qxx init.\n");

  W25QXX_DBG("get device id ... ");
  id = w25qxx_device_id_get();
  W25QXX_DBG("w25qxx id = %04X.\n", id);

  switch(id)
  {
    case 0xEF13:
    {
      _w25qxx_.id = W25Q08;
      _w25qxx_.sector_num = 32 * 8;
      break;
    }
    case 0xEF14:
    {
      _w25qxx_.id = W25Q16;
      _w25qxx_.sector_num = 32 * 16;
      break;
    }
    case 0xEF15:
    {
      _w25qxx_.id = W25Q32;
      _w25qxx_.sector_num = 32 * 32;
      break;
    }
    case 0xEF16:
    {
      _w25qxx_.id = W25Q64;
      _w25qxx_.sector_num = 32 * 64;
      break;
    }
    case 0xEF17:
    {
      _w25qxx_.id = W25Q128;
      _w25qxx_.sector_num = 32 * 128;
      break;
    }
    case 0xEF18:
    {
      _w25qxx_.id = W25Q256;
      _w25qxx_.sector_num = 32 * 256;
      break;
    }
    case 0xEF19:
    {
      _w25qxx_.id = W25Q512;
      _w25qxx_.sector_num = 32 * 512;
      break;
    }
    default:
      return 1;
  }

  if(device_reset())
  {
    return 1;
  }

  HAL_Delay(10);

  W25QXX_DBG("quad enable ... ");
  if(qspi_quad_enable())
  {
    W25QXX_DBG("fail.\n");
    return 1;
  }
  W25QXX_DBG("ok.\n");


  HAL_Delay(1);

  if(_w25qxx_.id <= W25Q128)
  {
    cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
  }
  else
  {
    cmd.AddressSize = HAL_OSPI_ADDRESS_32_BITS;
    qspi_enter_4byte_mode();
  }

  _w25qxx_.read = w25qxx_read;
  _w25qxx_.write = w25qxx_write;
  _w25qxx_.sector_erase = w25qxx_erase_sector;
  _w25qxx_.chip_erase = w25qxx_erase_chip;
  _w25qxx_.sector_size = 4 * 1024;

  w25qxx = &_w25qxx_;

  W25QXX_DBG("w25qxx init end.\n");
  //w25qxx_test();
  return 0;
}
