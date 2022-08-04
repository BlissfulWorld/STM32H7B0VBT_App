#ifndef _ANALOG_SPEAKER_H
#define _ANALOG_SPEAKER_H

#include "stdint.h"

typedef enum analog_spk_bits_e
{
  ANALOG_SPK_BITS_8,
  ANALOG_SPK_BITS_16,
}analog_spk_bits_et;

typedef uint32_t (*analog_speaker_get_data_func_t)(void* user, void* buffer, uint32_t size);
typedef void (*analog_speaker_play_finished_cb_func_t)(void* user);

int analog_speaker_play(uint32_t rate, analog_spk_bits_et bits, analog_speaker_get_data_func_t get_data, analog_speaker_play_finished_cb_func_t finished_cb, void* user);
int analog_speaker_is_busy(void);
void analog_speaker_volume_set(uint8_t volume);
uint8_t analog_speaker_volume_get(void);

#endif
