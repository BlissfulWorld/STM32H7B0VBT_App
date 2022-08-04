#include "main.h"
#include "ui.h"
#include "hw.h"
#include "config.h"

#include "adc.h"
#include "iwdg.h"
#include "spi.h"

#include "w25qxx.h"
#include "lcd.h"
#include "backlight.h"
#include "ip5306.h"
#include "key.h"
#include "pca9535.h"
#include "battery.h"
#include "analog_speaker.h"

#include "disk.h"
#include "cmsis_os.h"
#include "usb_device.h"
#include "test.h"
#include "test_board.h"
#include "wav.h"
#include "cJSON.h"
#include "user_settings.h"

#include "stdio.h"

uint8_t ucHeap[configTOTAL_HEAP_SIZE] __attribute__((section(".bss.RAM_D1")));

void mem_bss_clr(void)
{
  //#define TMP_VAR (*(uint32_t *)(0x20000000))
  int TMP_VAR;

  for(TMP_VAR = 0;TMP_VAR < 0xC0000;TMP_VAR++)
  {
    ((uint8_t*)(0x24040000))[TMP_VAR] = 0;
  }

  TMP_VAR = 0;
}

FILE __stdout;

void _sys_exit(int x)
{
  x = x;
}

int fputc(int ch, FILE* f)
{
  GUI_DispChar(ch);
  return ch;
}

void app_main(void* arg)
{
  uint8_t brightness,volume;
  uint32_t tick;

  MX_ADC1_Init();

  hw_init();
  
  us_load();

  test_init();

  ip5306_init();
  battery_init();
  w25qxx_init();
  external_storage_mount();
  HAL_IWDG_Refresh(&hiwdg1);
  IWDG1->KR = IWDG_KEY_RELOAD;

  lcd_init();
  ui_init();
  lcd_on();
  brightness = 100;
  US_GET(backlight,&brightness);
  backlight_set(brightness);
  backlight_open();
  tick = HAL_GetTick();
  
  volume = 10;
  US_GET(volume,&volume);
  analog_speaker_volume_set(volume);

  cJSON_Hooks hook = { .malloc_fn = pvPortMalloc, .free_fn = vPortFree };
  cJSON_InitHooks(&hook);

  key_init();

  while(HAL_GetTick() - tick < BOOT_LOGO_TIME_MIN) HAL_IWDG_Refresh(&hiwdg1);

  ui_home_open();

  ui_start();

  osThreadExit();
}
