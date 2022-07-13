#include "spiflash.h"
#include "spi.h"
#include "string.h"
#include "main.h"

#define PM25LQ_CS_CLR() HAL_GPIO_WritePin(SPI_FLASH_CS_GPIO_Port,SPI_FLASH_CS_Pin,GPIO_PIN_RESET)//do{}while(0) //
#define PM25LQ_CS_SET() HAL_GPIO_WritePin(SPI_FLASH_CS_GPIO_Port,SPI_FLASH_CS_Pin,GPIO_PIN_SET)//do{}while(0) //

int spiflash_product_id_read(uint8_t *manufacturer,uint8_t *device)
{
  uint8_t tx_buffer[6];
  uint8_t rx_buffer[6];
  
  PM25LQ_CS_CLR();

  memset(tx_buffer,0,sizeof(tx_buffer));
  memset(rx_buffer,0,sizeof(rx_buffer));
  
  tx_buffer[0] = 0x90;
  
  if(HAL_SPI_TransmitReceive(&hspi3,tx_buffer,rx_buffer,sizeof(tx_buffer),10) == HAL_OK)
  {
    *manufacturer = rx_buffer[4];
    *device = rx_buffer[5];
    PM25LQ_CS_SET();
    return 0;
  }

  PM25LQ_CS_SET();
  return -1;
}

int spiflash_uid_read(uint8_t *uid)
{
  uint8_t data[1];

  PM25LQ_CS_CLR();

  HAL_Delay(1);
  
  data[0] = 0x4B;
  HAL_SPI_Transmit(&hspi3,data,1,10);

  data[0] = 0x00;
  HAL_SPI_Transmit(&hspi3,data,1,10);
  HAL_SPI_Transmit(&hspi3,data,1,10);
  HAL_SPI_Transmit(&hspi3,data,1,10);
  HAL_SPI_Transmit(&hspi3,data,1,10);

  HAL_SPI_Receive(&hspi3,uid,16,160);

  PM25LQ_CS_SET();

  return 0;
}

int spiflash_read(uint32_t address,void *pdata,uint32_t size)
{
  uint32_t len;
  uint8_t data[4];

  for(uint32_t i = 0;i < size;i += len)
  {
    len = size - i;
    if(len > 1024) len = 1024;

    HAL_Delay(1);
    PM25LQ_CS_CLR();

    HAL_Delay(1);

    data[0] = 0x0B;
    HAL_SPI_Transmit(&hspi3,data,1,10);

    uint32_t tmp_address = address + i;
    data[0] = tmp_address >> 16;
    data[1] = tmp_address >> 8;
    data[2] = tmp_address;
    data[3] = 0x00;
    HAL_SPI_Transmit(&hspi3,data,4,10);

    HAL_SPI_Receive(&hspi3,(uint8_t*)pdata + i,len,len * 10);

    PM25LQ_CS_SET();
  }

  return 0;
}

static int read_status_register(uint8_t *value)
{
  int err;

  uint8_t instruction;

  err = 0;
  instruction = 0x05;

  PM25LQ_CS_CLR();

  if(HAL_SPI_Transmit(&hspi3,&instruction,1,10) || 
    HAL_SPI_Receive(&hspi3,value,1,10))
  {
    err = -1;
  }
  PM25LQ_CS_SET();

  return err;
}

static int write_enable(uint32_t timout)
{
  int err;
  uint8_t data;
  uint8_t instruction;
  uint32_t tick;

  err = 0;
  tick = HAL_GetTick();

  instruction = 0x06;

  PM25LQ_CS_CLR();

  if(HAL_SPI_Transmit(&hspi3,&instruction,1,10) != HAL_OK)
    err = -1;

  PM25LQ_CS_SET();

  if(err == 0)
  {
    while(1)
    {
      HAL_Delay(1);
      read_status_register(&data);
      if((data & 0x02) == 0x02)
        break;
      
      if(HAL_GetTick() - tick > timout)
      {
        err = -2;
        break;
      }
    }
  }
  
  return err;
}

int spiflash_write(uint32_t address,const void *pdata,uint32_t size,uint32_t timout)
{
  int err;
  uint32_t tick;
  uint32_t e_addr, s_addr, len;
  const uint8_t* p = pdata;
  uint8_t data[4];
  
  err = 0;
  tick = HAL_GetTick();

  e_addr = address + size;
  for(s_addr = address;s_addr < e_addr;s_addr += len)
  {
    len = e_addr - s_addr;
    if(len > 256)
      len = 256;

    if(write_enable(timout) != 0)
    {
      err = -1;
      break;
    }

    data[0] = 0x02;
    data[1] = s_addr >> 16;
    data[2] = s_addr >> 8;
    data[3] = s_addr;

    PM25LQ_CS_CLR();

    if((HAL_SPI_Transmit(&hspi3,data,4,10) != HAL_OK) || (HAL_SPI_Transmit(&hspi3,(uint8_t *)p,len,10) != HAL_OK))
      err = -2;

    PM25LQ_CS_SET();

    if(err == 0)
    {
      while(1)
      {
        HAL_Delay(1);
        read_status_register(data);
        if((data[0] & 0x01) == 0)
          break;
        
        if(HAL_GetTick() - tick > timout)
        {
          err = -2;
          break;
        }
      }
    }

    if(err != 0)
    {
      break;
    }

    p += len;
  }

  return err;
}

int spiflash_erase_sector(uint32_t address,uint32_t timout)
{
  int err;
  uint8_t data[4];
  uint32_t tick;

  err = 0;
  
  tick = HAL_GetTick();

  if(write_enable(timout) == 0)
  {
    data[0] = 0x20;
    data[1] = address >> 16;
    data[2] = address >> 8;
    data[3] = address;
    
    PM25LQ_CS_CLR();

    if(HAL_SPI_Transmit(&hspi3,data,4,10) != HAL_OK)
    {
      err = -2;
    }

    PM25LQ_CS_SET();

    if(err == 0)
    {
      while(1)
      {
        HAL_Delay(1);
        read_status_register(data);
        if((data[0] & 0x01) == 0)
          break;

        if(HAL_GetTick() - tick > timout)
        {
          err = -3;
          break;
        }
      }
    }
  }
  else
  {
    err = -1;
  }

  return err;
}

uint32_t spiflash_sector_num_get(uint8_t device)
{
  uint32_t number;
  
  number =  0;

  switch(device)
  {
    case 0x09:
    {
      number = 16;
      break;
    }
    case 0x10:
    {
      number = 32;
      break;
    }
    case 0x11:
    {
      number = 64;
      break;
    }
    case 0x12:
    {
      number = 128;
      break;
    }
    case 0x13:
    {
      number = 256;
      break;
    }
    case 0x14:
    {
      number = 512;
      break;
    }
    case 0x15:
    {
      number = 1024;
      break;
    }
    case 0x16:
    {
      number = 2048;
      break;
    }
    case 0x17:
    {
      number = 4096;
      break;
    }
    case 0x18:
    {
      number = 8192;
      break;
    }
    case 0x19:
    {
      number = 16384;
      break;
    }
  }

  return number;
}