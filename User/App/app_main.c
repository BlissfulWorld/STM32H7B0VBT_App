#include "main.h"
#include "stdio.h"
#include "ui.h"
#include "hw.h"
#include "config.h"
#include "disk.h"
#include "cmsis_os.h"
#include "usb_device.h"
#include "test.h"
#include "test_board.h"
#include "wav.h"
#include "cJSON.h"
#include "user_settings.h"
#include "sw_info.h"

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


FILE __stdout;

void _sys_exit(int x)
{
  x = x;
}

int fputc(int ch, FILE* f)
{
  char str[2];
  str[0] = ch;
  str[1] = 0;
  GUI_DispString(str);
  return ch;
}

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  SysTick->CTRL = 0;
  return HAL_OK;
}

void app_main(void* arg)
{
  uint8_t brightness, volume;

  MX_ADC1_Init();

  hw_init();

  us_load();

  test_init();

  ip5306_init();
  battery_init();
  w25qxx_init();
  external_storage_mount();
  IWDG1->KR = IWDG_KEY_RELOAD;

  lcd_init();
  ui_init();
  lcd_on();
  brightness = 100;
  US_GET(backlight, &brightness);
  backlight_set(brightness);
  backlight_open();

  volume = 10;
  US_GET(volume, &volume);
  analog_speaker_volume_set(volume);

  cJSON_Hooks hook = { .malloc_fn = pvPortMalloc, .free_fn = vPortFree };
  cJSON_InitHooks(&hook);

  key_init();

  if(battery_get_status() != BAT_STA_CHARGE)
  {
    ui_launch_open();
  }
  else
  {
    ui_charge_open();
  }

  ui_start();

  osThreadExit();
}
