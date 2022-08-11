#include "wav.h"
#include "ff.h"
#include "cmsis_os.h"
#include "string.h"
#include "analog_speaker.h"

typedef struct wav_play_s
{
  FIL fil;
  uint32_t size;
  analog_spk_bits_et bits;
}wav_play_st;

#define _4S2U32(tag) (uint32_t)(((tag)[0] << 0u) | ((tag)[1] << 8u) | ((tag)[2] << 16u) | ((tag)[3] << 24u))

#define ID_RIFF  _4S2U32("RIFF")
#define ID_FMT   _4S2U32("fmt ")
#define ID_DATA  _4S2U32("data")

#define FMT_WAVE _4S2U32("WAVE")

/**
 * @brief 音频数据填充回调
 *
 * @param wp wav_play_st结构体指针
 * @param buffer 需要填充的内存的指针
 * @param max_byte 最大的填充大小
 * @return uint32_t 已填充的字节数
 */
static uint32_t data_cb(void* wp, void* buffer, uint32_t max_byte)
{
  UINT br;
  uint32_t count;
  wav_play_st* wav_play;

  wav_play = wp;

  count = wav_play->size;
  if(count == 0)
    return 0;

  if(count > max_byte)
    count = max_byte;

  if(f_read(&wav_play->fil, buffer, count, &br))
  {
    return 0;
  }

  if(wav_play->bits != ANALOG_SPK_BITS_8)
  {
    //转换音频数据为无符号数据
    uint16_t* p = buffer;
    uint32_t len = br >> 1;
    for(uint32_t i = 0;i < len;i++)
    {
      p[i] += 0x8000;
    }
  }

  wav_play->size -= br;

  return br;
}

/**
 * @brief 播放完成
 *
 * @param wp wav_play_st结构体指针
 */
static void finish_cb(void* wp)
{
  wav_play_st* wav_play = wp;

  f_close(&wav_play->fil);
  vPortFree(wav_play);
}

/**
 * @brief 播放wav音频文件
 *        只能播放:
 *              通道数：1
 *              位数 ： 8bit或16bit
 *              采样率：小于44000Hz
 *
 * @param path 文件路径
 * @return int 错误码
 */
int wav_play(const char* path)
{
  int err;
  UINT br;
  wav_play_st* wav_play;
  FIL* fil;
  uint32_t count;

  uint8_t stage;

  chunk_st chunk;
  chunk_riff_st chunk_riff;
  chunk_fmt_st chunk_fmt;

  stage = 0;

  if(analog_speaker_is_busy())
  {
    err = -1;
    goto __exit;
  }

  wav_play = pvPortMalloc(sizeof(wav_play_st));
  if(wav_play == 0)
  {
    err = -2;
    goto __exit;
  }

  memset(wav_play, 0, sizeof(wav_play_st));

  fil = &wav_play->fil;

  if(f_open(fil, path, FA_READ) != FR_OK)
  {
    err = -3;
    goto __free_wav_play;
  }

  //读取第一个chunk
  if(f_read(fil, &chunk_riff, sizeof(chunk_riff), &br))
  {
    err = -4;
    goto __close;
  }

  //判断是否为wav文件
  if((chunk_riff.chunk.id != ID_RIFF) || (chunk_riff.format != FMT_WAVE))
  {
    err = -5;
    goto __close;
  }

  //查找格式信息和音频数据的位置
  while(1)
  {
    //读取chunk
    if(f_read(fil, &chunk, sizeof(chunk), &br))
    {
      err = -6;
      goto __close;
    }

    //判断chunk是否有效
    if((br != sizeof(chunk)) || (chunk.size == 0))
      break;

    //判断是否为格式chunk
    if(chunk.id == ID_FMT)
    {
      uint32_t size;
      chunk_fmt.chunk.id = chunk.id;
      chunk_fmt.chunk.size = chunk.size;

      //限制chunk大小，不能超出chunk_fmt_st的内容大小
      if(chunk_fmt.chunk.size > sizeof(chunk_fmt_st) - sizeof(chunk_st))
        size = sizeof(chunk_fmt_st) - sizeof(chunk_st);
      else
        size = chunk_fmt.chunk.size;

      //读取chunk内容
      if(f_read(fil, chunk_fmt.chunk.data, size, &br))
      {
        break;
      }

      //移动文件指针跳出这个chunk
      if(size < chunk_fmt.chunk.size)
      {
        f_lseek(fil, f_tell(fil) + (chunk_fmt.chunk.size - size));
      }

      stage |= 1;
    }
    //判断是否为数据chunk
    else if(chunk.id == ID_DATA)
    {
      count = chunk.size + 1;
      wav_play->size = count;

      stage |= 2;
    }
    else
    {
      //移动文件指针跳出这个chunk
      f_lseek(fil, f_tell(fil) + chunk.size);
    }

    //判断是否已经全部找到
    if(stage == 3)
    {
      break;
    }

    if(f_tell(fil) >= chunk_riff.chunk.size + 8)
      break;
  }

  //没全部找到则退出
  if(stage != 3)
  {
    err = -7;
    goto __close;
  }

  if((chunk_fmt.num_of_channels != 1) || ((chunk_fmt.bits_per_sample != 8) && (chunk_fmt.bits_per_sample != 16)) || (chunk_fmt.sample_rate > 44000))
  {
    err = -8;
    goto __close;
  }

  if(chunk_fmt.bits_per_sample == 8)
    wav_play->bits = ANALOG_SPK_BITS_8;
  else
    wav_play->bits = ANALOG_SPK_BITS_16;

  //开始播放
  if(analog_speaker_play(chunk_fmt.sample_rate, wav_play->bits, 1, data_cb, finish_cb, wav_play))
  {
    err = -9;
    goto __close;
  }

  err = 0;
  goto __exit;

__close:
  f_close(fil);

__free_wav_play:
  vPortFree(wav_play);

__exit:
  return err;
}
