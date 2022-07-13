#ifndef _KEY_H
#define _KEY_H

#include "stdint.h"

typedef enum key_event_e// : uint8_t
{
  KEY_RELEASE = (1 << 0),
  KEY_PRESS = (1 << 1),
  KEY_LONG_PRESS = (1 << 2),
  KEY_ONCLICK = (1 << 3),
  KEY_DBLCLICK = (1 << 4),
  KEY_REPET = (1 << 5),

  KEY_EVT_ENUM_END = (1 << 6)
}key_event_et;

typedef void (*key_event_cb_t)(uint8_t key, key_event_et evt);

#define KEY_NUM (10)

typedef enum key_num_e// : uint8_t
{
  KEY_S1, KEY_S2, KEY_S3, KEY_S4, KEY_S5, KEY_S6, KEY_S7, KEY_S8, KEY_S9
}key_num_et;

void key_init(void);
void key_deinit(void);
void key_set_event_cb(key_event_cb_t cb);
void key_task_init(void);
void key_proc(void);
uint8_t key_get(key_num_et* num, key_event_et* evt);
#endif
