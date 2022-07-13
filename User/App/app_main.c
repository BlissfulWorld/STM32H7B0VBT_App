#include "main.h"
#include "ui.h"
#include "hw.h"
#include "config.h"

#include "adc.h"
#include "iwdg.h"

#include "w25qxx.h"
#include "lcd.h"
#include "backlight.h"
#include "ip5306.h"
#include "key.h"
#include "pca9535.h"
#include "battery.h"

#include "disk.h"
#include "cmsis_os.h"
#include "usb_device.h"
#include "test.h"
#include "test_board.h"
#include "wav.h"
#include "cJSON.h"

#include "stdio.h"

#if MAKE_BOARD_LICENSE
static void make_license_to_board(void);
#endif

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

void print_err(int err)
{
  if(err)
    printf(" failure (%d).\n", err);
  else
    printf(" success.\n");
}

void app_main(void* arg)
{
  int i;
  uint32_t tick;

  MX_ADC1_Init();
  MX_ADC2_Init();

  hw_init();
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
  backlight_set(100);
  backlight_open();
  tick = HAL_GetTick();

  cJSON_Hooks hook = { .malloc_fn = pvPortMalloc, .free_fn = vPortFree };
  cJSON_InitHooks(&hook);

#if MAKE_BOARD_LICENSE
  //不返回
  make_license_to_board();
#endif

  key_init();

  while(HAL_GetTick() - tick < BOOT_LOGO_TIME_MIN) HAL_IWDG_Refresh(&hiwdg1);

  ui_home_open();

  ui_start();

  osThreadExit();
}

#if MAKE_BOARD_LICENSE

#if MAKE_BOARD_MODEL == XP7
  #define MODULE_ID (ip7_id)
  #define MODULE_NAME "XP7"
  #define CONFIG_FILE config_file_xp7_example
  #define CONFIG_FILE_SIZE sizeof(config_file_xp7_example)
#elif MAKE_BOARD_MODEL == XP7P
  #define MODULE_ID (ip7plus_id)
  #define MODULE_NAME "XP7Plus"
  #define CONFIG_FILE config_file_xp7plus_example
  #define CONFIG_FILE_SIZE sizeof(config_file_xp7plus_example)
#elif MAKE_BOARD_MODEL == XP8P
  #define MODULE_ID (ip8plus_id)
  #define MODULE_NAME "XP8Plus"
  #define CONFIG_FILE config_file_xp8plus_example
  #define CONFIG_FILE_SIZE sizeof(config_file_xp8plus_example)
#else
  #error "不支持的型号"
#endif

static void make_license_to_board(void)
{
  extern board_header_st* board_header_create(uint32_t module_id_code);
  extern int board_header_delete(board_header_st * _header_);
  extern int board_header_encrypt(board_header_st * _header_);
  extern int board_header_write(board_header_st * _header_);
  extern const test_module_id_ut ip7_id;
  extern const test_module_id_ut ip7plus_id;
  extern const test_module_id_ut ip8plus_id;

  board_header_st* header;
  uint8_t key, evt;


  GUI_Clear();
  GUI_DispString("\n\n\n");
  while(1)
  {
    int err;
    GUI_DispString("单击按键UP生成 " MODULE_NAME " License.\n");

    key_init();
    while(1)
    {
      if(key_get(&key, &evt) && (key == UI_KEY_UP) && (evt == KEY_ONCLICK))
      {
        break;
      }
      HAL_IWDG_Refresh(&hiwdg1);
    }
    key_deinit();

    GUI_Clear();

    GUI_DispString("\n\n\n检测中...\n");

    while(1)
    {
      header = board_header_create(MODULE_ID.code);
      if(header)
      {
        break;
      }

      osDelay(500);
      WRITE_REG(IWDG1->KR, IWDG_KEY_RELOAD);
    }

    GUI_DispString("发现板子...\n");

    printf("Manufacturer: %02X.", header->info.flash_manufacturer_id);
    GUI_DispString("\n");

    printf("Device: %02X.", header->info.flash_device_id);
    GUI_DispString("\n");

    printf("UID: ");
    for(int i = 0;i < 16;i++)
    {
      if(i && i % 10 == 0) GUI_DispString("\n");
      printf("%02X ", header->uid[i]);
    }
    GUI_DispString("\n");

    GUI_DispString("写入配置样例.\n");
    err = config_file_data_write_to_board(header, CONFIG_FILE, CONFIG_FILE_SIZE);

    if(err == 0)
    {
      GUI_DispString("生成 License.\n");
      board_header_encrypt(header);
    }

    if(err == 0)
    {
      GUI_DispString("写License进板子.\n");
      err = board_header_write(header);
    }

    if(err == 0)
    {
      GUI_DispString("成功.\n");
    }
    else
    {
      GUI_DispString("失败.\n");
    }
  }
}
#endif