#ifndef _WAV_H
#define _WAV_H

#include "stdint.h"

typedef struct __attribute__((packed)) chunk_s
{
  uint32_t id;
  uint32_t size;
  uint8_t data[0];
}chunk_st;

typedef struct __attribute__((packed)) chunk_riff_s
{
  chunk_st chunk;
  uint32_t format;
}chunk_riff_st;

typedef struct __attribute__((packed)) chunk_fmt_s
{
  chunk_st chunk;
  uint16_t audio_format;
  uint16_t num_of_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;
  //uint16_t byte_extra_data;
}chunk_fmt_st;

int wav_play(const char* path);

#endif
