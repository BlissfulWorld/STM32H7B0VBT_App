#ifndef _UI_H
#define _UI_H

#include "GUI.h"
#include "WM.h"
#include "multilanguage.h"
#include "font.h"
#include "resource.h"
#include "test.h"

typedef enum ui_key_num_e //: uint8_t
{
  UI_KEY_1, UI_KEY_2, UI_KEY_3, UI_KEY_4, UI_KEY_5, UI_KEY_6, UI_KEY_7, UI_KEY_8, UI_KEY_9, UI_KEY_10,
}ui_key_num_et;

typedef void (*ui_close_cb_func_t)(void*);

#define BACKGROUND_COLOR (0x696969)//深灰
//#define MENU_ITEM_SELECTED_COLOR (0xFF8C00)//深橙色

#define UI_KEY_EVT_RELEASE      (0x30)//48
#define UI_KEY_EVT_PRESS        (0x40)//64
#define UI_KEY_EVT_LONGPRESS    (0x50)//80
#define UI_KEY_EVT_ONCLICK      (0x80)//128
#define UI_KEY_EVT_DBLCLICK     (0xA0)//160
#define UI_KEY_EVT_REPET        (0xB0)//176


#define UI_KEY_UP     UI_KEY_1
#define UI_KEY_OK     UI_KEY_10
#define UI_KEY_DOWN   UI_KEY_3
#define UI_KEY_RIGHT  UI_KEY_5
#define UI_KEY_LEFT   UI_KEY_4

#define UI_KEY(num,evt)   (UI_KEY_##num + UI_KEY_EVT_##evt)

#define UI_MSG_BATTERY      (GUI_ID_USER + 0x53)
#define UI_MSG_ML           (GUI_ID_USER + 0x54)

#define UI_MSG_TEST         (GUI_ID_USER + 0x60)

#define UI_USBH_APP_DISCONNECT   (GUI_ID_USER + 0x70)
#define UI_USBH_APP_START        (GUI_ID_USER + 0x71)
#define UI_USBH_APP_READY        (GUI_ID_USER + 0x72)

int ui_init(void);
void ui_start(void);
void ui_show_logo(void);
int ui_screenshot(void);

void ui_battery_level_icon_updata(WM_HWIN image, uint8_t show_charge_lvl);

extern void ui_launch_open(void);
extern void ui_home_open(void);
extern void ui_config_open(void);
extern void ui_config_update_open(void);
extern void ui_nonsupport_open(void);
extern void ui_charge_open(void);
extern void ui_overcurrent_protection_open(ui_close_cb_func_t cb, void* user);

extern void ui_test_0_open(test_0_st* _test_, const char* _title_);
extern void ui_test_1_open(test_1_st* _test_, const char* _title_);
extern void ui_test_2_open(test_2_st* _test_, const char* _title_);
extern void ui_test_3_open(test_3_st* _test_, const char* _title_);


extern void ui_menu_open(ui_close_cb_func_t close_cb, void* user);
extern void ui_menu_brightness_open(WM_HWIN _back_);
extern void ui_menu_volume_open(WM_HWIN _back_);
extern void ui_menu_standby_open(WM_HWIN _back_);
extern void ui_menu_u_disk_mode_open(WM_HWIN _back_);
extern void ui_menu_os_test_open(WM_HWIN back);
extern void ui_menu_clone_app(WM_HWIN _back_);
extern void ui_menu_language_open(WM_HWIN back);
extern void ui_menu_about_open(WM_HWIN _back_);

extern void ui_menu_os_test_edit_open(void cb(void* p), void* p, void (*set) (uint32_t val), uint32_t(*get) (void), uint32_t(*convert) (uint32_t), void (*string) (char*, uint32_t, uint32_t));

extern void ui_os_test_result_open(const char* _name_, WM_HWIN _back_, const os_test_st* _os_test_, const float* _data_, uint8_t _len_);

#endif
