#include "font.h"
#include "ui.h"
#include "fatfs.h"
#include "stdio.h"
#include "string.h"

const GUI_FONT* font_ext;
static GUI_FONT font;
static GUI_XBF_DATA xbf_data;
static FIL font_fil;

/**
 * @brief 从xbf格式的字库文件中索引文字绘制数据
 *
 * @param off 文件偏移
 * @param num_byte 读取的字节数
 * @param arg 文件指针
 * @param pbuffer 存储文字绘制数据的缓存指针
 * @return int 错误码
 */
static int xbf_get_data(U32 off, U16 num_byte, void* arg, void* pbuffer)
{
  UINT br;
  FIL* fil = arg;

  f_lseek(fil, off);
  return f_read(fil, pbuffer, num_byte, &br);
}

/**
 * @brief 字体风格加载
 *
 * @return int 错误码
 */
int font_load(void)
{
  char path[24];

  snprintf(path, sizeof(path), "%sfont/font.bin", DISK_SYS_PATH);

  if(f_open(&font_fil, path, FA_READ) == FR_OK)
  {
    GUI_XBF_CreateFont(&font, &xbf_data, GUI_XBF_TYPE_PROP, xbf_get_data, &font_fil);
    font_ext = &font;
  }
  else
  {
    font_ext = GUI_DEFAULT_FONT;
  }

  return 0;
}
