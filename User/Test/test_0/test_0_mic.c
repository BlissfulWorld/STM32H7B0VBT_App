#include "test.h"
#include "string.h"
//#include "analog_mic.h"
#include "digital_mic.h"
#include "cmsis_os.h"
#include "analog_speaker.h"

#define SAMPLE_RATE_HZ        (16000)
#define MIC_RECORD_MAX_LENGTH (SAMPLE_RATE_HZ * 10)//10Ãë

#define STAGE_IDLE       (0)
#define STAGE_RECORDING  (1)
#define STAGE_FINISHED   (2)

static uint8_t mic;
static uint16_t* mic_record;
static uint32_t record_count;
static uint8_t stage;
static uint32_t play_count;

int test_0_mic_init(void)
{
  if(mic_record == 0)
  {
    mic_record = pvPortMalloc(MIC_RECORD_MAX_LENGTH * 2);
    if(mic_record)
    {
      memset(mic_record, 0, MIC_RECORD_MAX_LENGTH * 2);
    }
  }

  stage = 0;
  record_count = 0;

  if(digital_mic_start(SAMPLE_RATE_HZ))
    return -1;

  return 0;
}

uint8_t test_0_mic_get_percentage(void)
{
  uint16_t value;

  value = mic;
  value = (double)value * 100 / 256.0;

  if(value > 100) value = 100;

  return value;
}

void test_0_mic_update_data(void)
{
  mic = digital_mic_get_current_value();
}

static void test_0_mic_record_cb(void)
{
  record_count = MIC_RECORD_MAX_LENGTH;
  stage = STAGE_FINISHED;
}

int test_0_mic_record_start(void)
{
  if(mic_record)
  {
    play_count = 0;
    stage = STAGE_RECORDING;
    digital_mic_record_stop();
    digital_mic_record_start(mic_record, MIC_RECORD_MAX_LENGTH, test_0_mic_record_cb);
    return 0;
  }
  else
  {
    return -1;
  }
}

int test_0_mic_record_stop(void)
{
  stage = STAGE_FINISHED;
  record_count = MIC_RECORD_MAX_LENGTH - digital_mic_record_get_count();
  digital_mic_record_stop();
  return 0;
}

uint8_t test_0_mic_record_get_progress(void)
{
  if(stage == STAGE_RECORDING)
    return (MIC_RECORD_MAX_LENGTH - digital_mic_record_get_count()) * 100 / MIC_RECORD_MAX_LENGTH;
  else
    return 100;
}

static uint32_t data_cb(void* p, void* buffer, uint32_t max)
{
  uint32_t count;
  uint8_t* record;
  uint8_t* out;

  if(stage == STAGE_FINISHED)
  {
    count = (record_count << 1) - play_count;

    if(count)
    {
      if(count > max)
        count = max;

      record = (uint8_t*)mic_record + play_count;

      out = buffer;
      for(uint32_t i = 0;i < count;i++)
      {
        out[i] = record[i];
      }

      play_count += count;

      return count;
    }
  }

  play_count = 0;

  return 0;
}

int test_0_mic_record_play(void)
{
  if(stage)
  {
    if(stage == STAGE_FINISHED)
    {
      return analog_speaker_play(SAMPLE_RATE_HZ, ANALOG_SPK_BITS_16, data_cb, 0, 0) ? -3 : 0;
    }

    return -2;
  }

  return -1;
}
