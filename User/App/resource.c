#include "resource.h"
#include "fatfs.h"
#include "stdio.h"

resource_icon_st* resource_icon_load(const char *name)
{
  FRESULT res;
  FIL *fp;
  UINT br;
  resource_icon_st* src_p;
  char path[128];
  
  src_p = 0;

  fp = (FIL *)pvPortMalloc(sizeof(FIL));
  if(fp == 0)
  {
    goto __exit;
  }

  snprintf(path, sizeof(path), "%sicon/%s.dta",USERPath,name);

  res = f_open(fp, path, FA_READ);
  if(res != FR_OK)
  {
    goto __free_fp;
  }

  src_p = (resource_icon_st*)pvPortMalloc(sizeof(resource_icon_st) + f_size(fp));
  if(src_p == 0)
  {
    goto __close_fp;
  }
  
  src_p->size = f_size(fp);

  res = f_read(fp, src_p->data, src_p->size, &br);
  if((res != FR_OK) || (br != src_p->size))
  {
    goto __free_src_p;
  }

  goto __close_fp;

__free_src_p:
  vPortFree(src_p);
  src_p = 0;

__close_fp:
  f_close(fp);

__free_fp:
  vPortFree(fp);

__exit:
  return src_p;
}

int resource_icon_unload(resource_icon_st *icon)
{
  if(icon)
  {
    vPortFree(icon);
    return 0;
  }
  else
  {
    return -1;
  }
}