#include "font.h"
#include "ui.h"
#include "ff.h"
#include "stdio.h"
#include "string.h"

const GUI_FONT *font_ext;
static GUI_FONT font;
static GUI_XBF_DATA xbf_data;
static FIL font_fil;

static int xbf_get_data(U32 off,U16 num_byte,void *arg,void *pbuffer)
{
  UINT br;
  FIL *fil = arg;
  
  f_lseek(fil,off);
  return f_read(fil,pbuffer,num_byte,&br);
}

uint8_t font_load(void)
{
  if(f_open(&font_fil,"0:/font/font.bin",FA_READ) == FR_OK)
  {
    GUI_XBF_CreateFont(&font,&xbf_data,GUI_XBF_TYPE_PROP,xbf_get_data,&font_fil);
    font_ext = &font;
  }
  else
  {
    font_ext = GUI_FONT_8X16;
  }

  return 0;
}
