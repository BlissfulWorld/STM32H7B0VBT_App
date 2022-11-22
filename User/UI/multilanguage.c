#include "multilanguage.h"
#include "string.h"
#include "ff.h"
#include "stdio.h"

#if 0
#include "out.h"
#define ML_DBG(...) printf(__VA_ARGS__)
#else
#define ML_DBG(...) do{}while(0)
#endif

typedef struct ml_str_file_dta_s
{
  unsigned short id;
  char str[0];
}ml_str_file_dta_st;

typedef struct ml_str_file_s
{
  char head[4];
  uint32_t size;
  uint8_t dummy[20];
  ml_str_file_dta_st data[0];
}ml_str_file_st;

struct ui_str_s ui_str __attribute((section(".AHB_SRAM2")));

typedef struct ml_str_id_s
{
  const char** p;
  ml_id_et id;
}ml_str_id_st;

const ml_str_id_st ml_str_id[] =
{
  {&ui_str.name,ML_NAME},

  {&ui_str.ui_lanuch.update_resource,ML_UI_LAUNCH_UPDATE_RESOURCE},

  {&ui_str.ui_charge.full,ML_UI_CHARGE_FULL},
  {&ui_str.ui_charge.charging,ML_UI_CHARGE_CHARGING},
  {&ui_str.ui_charge.discharging,ML_UI_CHARGE_DISCHARGING},

  {&ui_str.ui_home.title,ML_UI_HOME_TITIL},
  {&ui_str.ui_home.btn_update,ML_UI_HOME_BTN_UPDATE},
  {&ui_str.ui_home.btn_start,ML_UI_HOME_BTN_START},
  {&ui_str.ui_home.btn_menu,ML_UI_HOME_BTN_MENU},
  {&ui_str.ui_home.waiting,ML_UI_HOME_WAITING},
  {&ui_str.ui_home.name,ML_UI_HOME_NAME},
  {&ui_str.ui_home.version,ML_UI_HOME_VERSION},
  {&ui_str.ui_home.description,ML_UI_HOME_DESC},

  {&ui_str.ui_config_update.title,ML_UI_CONFIG_UPDATE_TITLE},
  {&ui_str.ui_config_update.btn_back,ML_UI_CONFIG_UPDATE_BTN_BACK},
  {&ui_str.ui_config_update.btn_update,ML_UI_CONFIG_UPDATE_BTN_UPDATE},
  {&ui_str.ui_config_update.name,ML_UI_CONFIG_UPDATE_NAME},
  {&ui_str.ui_config_update.version,ML_UI_CONFIG_UPDATE_VERSION},
  {&ui_str.ui_config_update.description,ML_UI_CONFIG_UPDATE_DESC},
  {&ui_str.ui_config_update.now_config,ML_UI_CONFIG_UPDATE_NOW_CONFIG},
  {&ui_str.ui_config_update.new_config,ML_UI_CONFIG_UPDATE_NEW_CONFIG},
  {&ui_str.ui_config_update.updating,ML_UI_CONFIG_UPDATE_UPDATING},
  {&ui_str.ui_config_update.update_finished,ML_UI_CONFIG_UPDATE_UPDATE_FINISHED},
  {&ui_str.ui_config_update.update_failed,ML_UI_CONFIG_UPDATE_UPDATE_FAILED},

  {&ui_str.ui_ostest_result.title,ML_UI_OSTEST_RESULT_TITLE},
  {&ui_str.ui_ostest_result.btn_back,ML_UI_OSTEST_RESULT_BTN_BACK},
  {&ui_str.ui_ostest_result.btn_exp_data,ML_UI_OSTEST_RESULT_BTN_EXPORT_DATA},
  {&ui_str.ui_ostest_result.btn_learn,ML_UI_OSTEST_RESULT_BTN_LEARN},
  {&ui_str.ui_ostest_result.btn_no,ML_UI_OSTEST_RESULT_BTN_NO},
  {&ui_str.ui_ostest_result.btn_yes,ML_UI_OSTEST_RESULT_BTN_YES},
  {&ui_str.ui_ostest_result.exporting,ML_UI_OSTEST_RESULT_EXPORTING},
  {&ui_str.ui_ostest_result.export_finished,ML_UI_OSTEST_RESULT_EXPORT_FINISHED},
  {&ui_str.ui_ostest_result.export_failed,ML_UI_OSTEST_RESULT_EXPORT_FAILED},
  {&ui_str.ui_ostest_result.export_data_to_storage,ML_UI_OSTEST_RESULT_EXPORT_DATA_TO_STORAGE},
  {&ui_str.ui_ostest_result.learning,ML_UI_OSTEST_RESULT_LEARNING},
  {&ui_str.ui_ostest_result.learn_finished,ML_UI_OSTEST_RESULT_LEARN_FINISHED},
  {&ui_str.ui_ostest_result.learn_failed,ML_UI_OSTEST_RESULT_LEARN_FAILED},
  {&ui_str.ui_ostest_result.learn_and_write_to_flash,ML_UI_OSTEST_RESULT_LEARN_AND_WRITE_FLASH},

  {&ui_str.ui_nonsupport.title,ML_UI_NONSUPPORT_TITLE},
  {&ui_str.ui_nonsupport.info,ML_UI_NONSUPPORT_INFO},

  {&ui_str.ui_menu.title,ML_UI_MENU_TITLE},
  {&ui_str.ui_menu.btn_back,ML_UI_MENU_BTN_BACK},
  {&ui_str.ui_menu.btn_confirm,ML_UI_MENU_BTN_CONFIRM},
  {&ui_str.ui_menu.brightness,ML_UI_MENU_BRIGHTNESS},
  {&ui_str.ui_menu.volume,ML_UI_MENU_VOLUME},
  {&ui_str.ui_menu.standby,ML_UI_MENU_STANDBY},
  {&ui_str.ui_menu.u_disk_mode,ML_UI_MENU_U_DISK_MODE},
  {&ui_str.ui_menu.os_test,ML_UI_MENU_OSTEST},
  {&ui_str.ui_menu.clone_app,ML_UI_MENU_CLONE_APP},
  {&ui_str.ui_menu.language,ML_UI_MENU_LANGUAGE},
  {&ui_str.ui_menu.about,ML_UI_MENU_ABOUT},

  {&ui_str.ui_m_brightness.title,ML_UI_M_BRIGHTNESS_TITLE},
  {&ui_str.ui_m_brightness.btn_back,ML_UI_M_BRIGHTNESS_BTN_BACK},
  {&ui_str.ui_m_brightness.btn_save,ML_UI_M_BRIGHTNESS_BTN_SAVE},

  {&ui_str.ui_m_volume.title,ML_UI_M_VOLUME_TITLE},
  {&ui_str.ui_m_volume.btn_back,ML_UI_M_VOLUME_BTN_BACK},
  {&ui_str.ui_m_volume.btn_play,ML_UI_M_VOLUME_BTN_PLAY},
  {&ui_str.ui_m_volume.btn_save,ML_UI_M_VOLUME_BTN_SAVE},

  {&ui_str.ui_m_standby.title,ML_UI_M_STANDBY_TITLE},
  {&ui_str.ui_m_standby.btn_back,ML_UI_M_STANDBY_BTN_BACK},
  {&ui_str.ui_m_standby.btn_save,ML_UI_M_STANDBY_BTN_SAVE},

  {&ui_str.ui_m_u_disk_mode.title,ML_UI_M_U_DISK_MODE_TITLE},
  {&ui_str.ui_m_u_disk_mode.btn_back,ML_UI_M_U_DISK_MODE_BTN_BACK},
  {&ui_str.ui_m_u_disk_mode.info,ML_UI_M_U_DISK_MODE_INFO},

  {&ui_str.ui_m_ostest.title,ML_UI_M_OSTEST_TITLE},
  {&ui_str.ui_m_ostest.btn_back,ML_UI_M_OSTEST_BTN_BACK},
  {&ui_str.ui_m_ostest.btn_edit,ML_UI_M_OSTEST_BTN_EDIT},
  {&ui_str.ui_m_ostest.function,ML_UI_M_OSTEST_FUNCTION},
  {&ui_str.ui_m_ostest.function_enable,ML_UI_M_OSTEST_FUNCTION_ENABLE},
  {&ui_str.ui_m_ostest.function_disable,ML_UI_M_OSTEST_FUNCTION_DISABLE},
  {&ui_str.ui_m_ostest.max_error,ML_UI_M_OSTEST_MAX_ERROR},
  {&ui_str.ui_m_ostest_edit.btn_cancel,ML_UI_M_OSTEST_EDIT_CANCEL},
  {&ui_str.ui_m_ostest_edit.btn_confirm,ML_UI_M_OSTEST_EDIT_CONFIRM},

  {&ui_str.ui_m_clone.title,ML_UI_M_CLONE_APP_TITLE},
  {&ui_str.ui_m_clone.btn_back,ML_UI_M_CLONE_BTN_BACK},
  {&ui_str.ui_m_clone.btn_start,ML_UI_M_CLONE_BTN_START},
  {&ui_str.ui_m_clone.software_version,ML_UI_M_CLONE_SOFTWARE_VERSION},
  {&ui_str.ui_m_clone.device_disconnect,ML_UI_M_CLONE_DEVICE_DISCONNECT},
  {&ui_str.ui_m_clone.connecting,ML_UI_M_CLONE_CONNECTING},
  {&ui_str.ui_m_clone.connected,ML_UI_M_CLONE_CONNECTED},
  {&ui_str.ui_m_clone.error,ML_UI_M_CLONE_ERROR},
  {&ui_str.ui_m_clone.clone_failed,ML_UI_M_CLONE_CLONE_FAILED},
  {&ui_str.ui_m_clone.clone_finished,ML_UI_M_CLONE_CLONE_FINISHED},
  {&ui_str.ui_m_clone.start_clone,ML_UI_M_CLONE_START_CLONE},

  {&ui_str.ui_m_language.title,ML_UI_M_LANGUAGE_TITLE},
  {&ui_str.ui_m_language.btn_back,ML_UI_M_LANGUAGE_BTN_BACK},
  {&ui_str.ui_m_language.btn_select,ML_UI_M_LANGUAGE_BTN_SELECT},
  {&ui_str.ui_m_language.btn_save,ML_UI_M_LANGUAGE_BTN_SAVE},

  {&ui_str.ui_m_about.title,ML_UI_M_ABOUT_TITLE},
  {&ui_str.ui_m_about.btn_back,ML_UI_M_ABOUT_BTN_BACK},
  {&ui_str.ui_m_about.app_version,ML_UI_M_ABOUT_APP_VERSION},
  {&ui_str.ui_m_about.bootloader_version,ML_UI_M_ABOUT_BOOTLOADER_VERSION},

  {&ui_str.ui_test_0.title,ML_UI_TEST_0_TITLE},
  {&ui_str.ui_test_0.btn_menu,ML_UI_TEST_0_BTN_MENU},
  {&ui_str.ui_test_0.btn_record_play,ML_UI_TEST_0_BTN_RECORD_PLAY},
  {&ui_str.ui_test_0.info_no_found_module,ML_UI_TEST_0_INFO_NO_FOUND_MODULE},
  {&ui_str.ui_test_0.info_doing_ostest,ML_UI_TEST_0_INFO_DOING_OSTEST},
  {&ui_str.ui_test_0.prox,ML_UI_TEST_0_PROX},
  {&ui_str.ui_test_0.als,ML_UI_TEST_0_ALS},
  {&ui_str.ui_test_0.mic,ML_UI_TEST_0_MIC},
  {&ui_str.ui_test_0.ostest,ML_UI_TEST_0_OSTEST},
  {&ui_str.ui_test_0.camera,ML_UI_TEST_0_CAMERA},
  {&ui_str.ui_test_0.camera_found,ML_UI_TEST_0_CAMERA_FOUND},
  {&ui_str.ui_test_0.camera_no_found,ML_UI_TEST_0_CAMERA_NO_FOUND},
  {&ui_str.ui_test_0.spk_p,ML_UI_TEST_0_SPK_P},
  {&ui_str.ui_test_0.spk_n,ML_UI_TEST_0_SPK_N},
  {&ui_str.ui_test_0.ostest_result_normal,ML_UI_TEST_0_OSTEST_RESULT_NORMAL},
  {&ui_str.ui_test_0.ostest_result_exception,ML_UI_TEST_0_OSTEST_RESULT_EXCEPTION},
  {&ui_str.ui_test_0.key_up_check,ML_UI_TEST_0_KEY_UP_CHECK},
  {&ui_str.ui_test_0.no_ostest,ML_UI_TEST_0_NO_OSTEST},
  {&ui_str.ui_test_0.info_device_exception,ML_UI_TEST_0_INFO_DEVICE_EXCEPTION},

  {&ui_str.ui_test_1.title,ML_UI_TEST_1_TITLE},
  {&ui_str.ui_test_1.btn_speaker_play,ML_UI_TEST_1_BTN_SPEAKER_PLAY},
  {&ui_str.ui_test_1.btn_menu,ML_UI_TEST_1_BTN_MENU},
  {&ui_str.ui_test_1.btn_record_play,ML_UI_TEST_1_BTN_RECORD_PLAY},
  {&ui_str.ui_test_1.info_no_found_module,ML_UI_TEST_1_INFO_NO_FOUND_MODULE},
  {&ui_str.ui_test_1.info_doing_ostest,ML_UI_TEST_1_INFO_DOING_OSTEST},
  {&ui_str.ui_test_1.prox,ML_UI_TEST_1_PROX},
  {&ui_str.ui_test_1.als,ML_UI_TEST_1_ALS},
  {&ui_str.ui_test_1.mic,ML_UI_TEST_1_MIC},
  {&ui_str.ui_test_1.ostest,ML_UI_TEST_1_OSTEST},
  {&ui_str.ui_test_1.spk_p,ML_UI_TEST_1_SPK_P},
  {&ui_str.ui_test_1.spk_n,ML_UI_TEST_1_SPK_N},
  {&ui_str.ui_test_1.cathc,ML_UI_TEST_1_CATHC},
  {&ui_str.ui_test_1.yogi,ML_UI_TEST_1_YOGI},
  {&ui_str.ui_test_1.hall,ML_UI_TEST_1_HALL},
  {&ui_str.ui_test_1.ostest_result_normal,ML_UI_TEST_1_OSTEST_RESULT_NORMAL},
  {&ui_str.ui_test_1.ostest_result_exception,ML_UI_TEST_1_OSTEST_RESULT_EXCEPTION},
  {&ui_str.ui_test_1.key_up_check,ML_UI_TEST_1_KEY_UP_CHECK},
  {&ui_str.ui_test_1.no_ostest,ML_UI_TEST_1_NO_OSTEST},
  {&ui_str.ui_test_1.info_device_exception,ML_UI_TEST_1_INFO_DEVICE_EXCEPTION},

  {&ui_str.ui_test_2.title,ML_UI_TEST_2_TITLE},
  {&ui_str.ui_test_2.btn_speaker_play,ML_UI_TEST_2_BTN_SPEAKER_PLAY},
  {&ui_str.ui_test_2.btn_menu,ML_UI_TEST_2_BTN_MENU},
  {&ui_str.ui_test_2.btn_record_play,ML_UI_TEST_2_BTN_RECORD_PLAY},
  {&ui_str.ui_test_2.info_no_found_module,ML_UI_TEST_2_INFO_NO_FOUND_MODULE},
  {&ui_str.ui_test_2.info_doing_ostest,ML_UI_TEST_2_INFO_DOING_OSTEST},
  {&ui_str.ui_test_2.prox,ML_UI_TEST_2_PROX},
  {&ui_str.ui_test_2.als,ML_UI_TEST_2_ALS},
  {&ui_str.ui_test_2.mic,ML_UI_TEST_2_MIC},
  {&ui_str.ui_test_2.ostest,ML_UI_TEST_2_OSTEST},
  {&ui_str.ui_test_2.spk_p,ML_UI_TEST_2_SPK_P},
  {&ui_str.ui_test_2.spk_n,ML_UI_TEST_2_SPK_N},
  {&ui_str.ui_test_2.cathc,ML_UI_TEST_2_CATHC},
  {&ui_str.ui_test_2.yogi,ML_UI_TEST_2_YOGI},
  {&ui_str.ui_test_2.ostest_result_normal,ML_UI_TEST_2_OSTEST_RESULT_NORMAL},
  {&ui_str.ui_test_2.ostest_result_exception,ML_UI_TEST_2_OSTEST_RESULT_EXCEPTION},
  {&ui_str.ui_test_2.key_up_check,ML_UI_TEST_2_KEY_UP_CHECK},
  {&ui_str.ui_test_2.no_ostest,ML_UI_TEST_2_NO_OSTEST},
  {&ui_str.ui_test_2.info_device_exception,ML_UI_TEST_2_INFO_DEVICE_EXCEPTION},

  {&ui_str.ui_test_3.title,ML_UI_TEST_3_TITLE},
  {&ui_str.ui_test_3.btn_speaker_play,ML_UI_TEST_3_BTN_SPEAKER_PLAY},
  {&ui_str.ui_test_3.btn_menu,ML_UI_TEST_3_BTN_MENU},
  {&ui_str.ui_test_3.btn_record_play,ML_UI_TEST_3_BTN_RECORD_PLAY},
  {&ui_str.ui_test_3.info_no_found_module,ML_UI_TEST_3_INFO_NO_FOUND_MODULE},
  {&ui_str.ui_test_3.info_doing_ostest,ML_UI_TEST_3_INFO_DOING_OSTEST},
  {&ui_str.ui_test_3.prox,ML_UI_TEST_3_PROX},
  {&ui_str.ui_test_3.als,ML_UI_TEST_3_ALS},
  {&ui_str.ui_test_3.mic,ML_UI_TEST_3_MIC},
  {&ui_str.ui_test_3.ostest,ML_UI_TEST_3_OSTEST},
  {&ui_str.ui_test_3.spk_p,ML_UI_TEST_3_SPK_P},
  {&ui_str.ui_test_3.spk_n,ML_UI_TEST_3_SPK_N},
  {&ui_str.ui_test_3.cathc,ML_UI_TEST_3_CATHC},
  {&ui_str.ui_test_3.yogi,ML_UI_TEST_3_YOGI},
  {&ui_str.ui_test_3.compass,ML_UI_TEST_3_COMPASS},
  {&ui_str.ui_test_3.ostest_result_normal,ML_UI_TEST_3_OSTEST_RESULT_NORMAL},
  {&ui_str.ui_test_3.ostest_result_exception,ML_UI_TEST_3_OSTEST_RESULT_EXCEPTION},
  {&ui_str.ui_test_3.key_up_check,ML_UI_TEST_3_KEY_UP_CHECK},
  {&ui_str.ui_test_3.no_ostest,ML_UI_TEST_3_NO_OSTEST},
  {&ui_str.ui_test_3.info_device_exception,ML_UI_TEST_3_INFO_DEVICE_EXCEPTION},

  {&ui_str.ui_test_4.title,ML_UI_TEST_4_TITLE},
  {&ui_str.ui_test_4.btn_menu,ML_UI_TEST_4_BTN_MENU},
  {&ui_str.ui_test_4.btn_record_play,ML_UI_TEST_4_BTN_RECORD_PLAY},
  {&ui_str.ui_test_4.info_no_found_module,ML_UI_TEST_4_INFO_NO_FOUND_MODULE},
  {&ui_str.ui_test_4.info_doing_ostest,ML_UI_TEST_4_INFO_DOING_OSTEST},
  {&ui_str.ui_test_4.mic1,ML_UI_TEST_4_MIC1},
  {&ui_str.ui_test_4.mic1_found,ML_UI_TEST_4_MIC1_FOUND},
  {&ui_str.ui_test_4.mic1_no_found,ML_UI_TEST_4_MIC1_NO_FOUND},
  {&ui_str.ui_test_4.mic,ML_UI_TEST_4_MIC},
  {&ui_str.ui_test_4.ostest,ML_UI_TEST_4_OSTEST},
  {&ui_str.ui_test_4.ostest_result_normal,ML_UI_TEST_4_OSTEST_RESULT_NORMAL},
  {&ui_str.ui_test_4.ostest_result_oc,ML_UI_TEST_4_OSTEST_RESULT_OC},
  {&ui_str.ui_test_4.ostest_result_sc,ML_UI_TEST_4_OSTEST_RESULT_SC},
  {&ui_str.ui_test_4.ostest_result_total,ML_UI_TEST_4_OSTEST_RESULT_TOTAL},
  {&ui_str.ui_test_4.info_device_exception,ML_UI_TEST_4_INFO_DEVICE_EXCEPTION},
};

static void language_cn_config(void)
{
  ui_str.name = "简体中文";

  ui_str.ui_lanuch.update_resource = "更新资源...";

  ui_str.ui_charge.charging = "充电中...";
  ui_str.ui_charge.full = "已充满";
  ui_str.ui_charge.discharging = "未充电";

  ui_str.ui_home.title = "配置";
  ui_str.ui_home.btn_update = "更新";
  ui_str.ui_home.btn_start = "开始";
  ui_str.ui_home.btn_menu = "菜单";
  ui_str.ui_home.waiting = "等待小板";
  ui_str.ui_home.name = "名称";
  ui_str.ui_home.version = "版本";
  ui_str.ui_home.description = "描述";

  ui_str.ui_config_update.title = "更新小板配置";
  ui_str.ui_config_update.btn_back = "返回";
  ui_str.ui_config_update.btn_update = "更新";
  ui_str.ui_config_update.name = "名称";
  ui_str.ui_config_update.version = "版本";
  ui_str.ui_config_update.description = "描述";
  ui_str.ui_config_update.now_config = "当前配置";
  ui_str.ui_config_update.new_config = "新配置";
  ui_str.ui_config_update.updating = "更新中...";
  ui_str.ui_config_update.update_finished = "更新完成";
  ui_str.ui_config_update.update_failed = "更新失败";

  ui_str.ui_ostest_result.title = "开短路测试结果";
  ui_str.ui_ostest_result.btn_back = "返回";
  ui_str.ui_ostest_result.btn_exp_data = "导出数据";
  ui_str.ui_ostest_result.btn_learn = "学习";
  ui_str.ui_ostest_result.btn_no = "否";
  ui_str.ui_ostest_result.btn_yes = "是";
  ui_str.ui_ostest_result.exporting = "导出中...";
  ui_str.ui_ostest_result.export_finished = "导出完成";
  ui_str.ui_ostest_result.export_failed = "导出失败";
  ui_str.ui_ostest_result.export_data_to_storage = "导出数据到存储？";
  ui_str.ui_ostest_result.learning = "学习中...";
  ui_str.ui_ostest_result.learn_finished = "已学习到小板中";
  ui_str.ui_ostest_result.learn_failed = "失败";
  ui_str.ui_ostest_result.learn_and_write_to_flash = "学习并写入小板？";

  ui_str.ui_nonsupport.title = "错误";
  ui_str.ui_nonsupport.info = "不支持！";

  ui_str.ui_menu.title = "菜单";
  ui_str.ui_menu.btn_back = "返回";
  ui_str.ui_menu.btn_confirm = "确定";
  ui_str.ui_menu.brightness = "亮度";
  ui_str.ui_menu.volume = "系统音量";
  ui_str.ui_menu.standby = "待机";
  ui_str.ui_menu.u_disk_mode = "U盘模式";
  ui_str.ui_menu.os_test = "开短路";
  ui_str.ui_menu.clone_app = "克隆";
  ui_str.ui_menu.language = "语言";
  ui_str.ui_menu.about = "关于";

  ui_str.ui_m_brightness.title = "亮度";
  ui_str.ui_m_brightness.btn_back = "返回";
  ui_str.ui_m_brightness.btn_save = "保存";

  ui_str.ui_m_volume.title = "系统音量";
  ui_str.ui_m_volume.btn_back = "返回";
  ui_str.ui_m_volume.btn_play = "播放";
  ui_str.ui_m_volume.btn_save = "保存";

  ui_str.ui_m_standby.title = "待机多少分钟关机";
  ui_str.ui_m_standby.btn_back = "返回";
  ui_str.ui_m_standby.btn_save = "保存";

  ui_str.ui_m_u_disk_mode.title = "U盘模式";
  ui_str.ui_m_u_disk_mode.btn_back = "返回";
  ui_str.ui_m_u_disk_mode.info = "使用USB接口将文件复制到内置存储\n退出须长按返回键\n";

  ui_str.ui_m_ostest.title = "开短路";
  ui_str.ui_m_ostest.btn_back = "返回";
  ui_str.ui_m_ostest.btn_edit = "编辑";
  ui_str.ui_m_ostest.function = "功能";
  ui_str.ui_m_ostest.function_enable = "启用";
  ui_str.ui_m_ostest.function_disable = "关闭";
  ui_str.ui_m_ostest.max_error = "最大误差";

  ui_str.ui_m_clone.title = "克隆";
  ui_str.ui_m_clone.btn_back = "返回";
  ui_str.ui_m_clone.btn_start = "开始克隆";
  ui_str.ui_m_clone.software_version = "软件版本";
  ui_str.ui_m_clone.device_disconnect = "设备未连接";
  ui_str.ui_m_clone.connecting = "连接中...";
  ui_str.ui_m_clone.connected = "已连接";
  ui_str.ui_m_clone.error = "错误";
  ui_str.ui_m_clone.clone_failed = "克隆失败";
  ui_str.ui_m_clone.clone_finished = "克隆完成";
  ui_str.ui_m_clone.start_clone = "开始克隆";

  ui_str.ui_m_language.title = "语言";
  ui_str.ui_m_language.btn_back = "返回";
  ui_str.ui_m_language.btn_select = "选择";
  ui_str.ui_m_language.btn_save = "保存";

  ui_str.ui_m_about.title = "关于";
  ui_str.ui_m_about.btn_back = "返回";
  ui_str.ui_m_about.app_version = "软件版本";
  ui_str.ui_m_about.bootloader_version = "底层版本";

  ui_str.ui_m_ostest_edit.btn_cancel = "取消";
  ui_str.ui_m_ostest_edit.btn_confirm = "确定";

  ui_str.ui_test_0.title = "";
  ui_str.ui_test_0.btn_menu = "菜单";
  ui_str.ui_test_0.btn_record_play = "录/播";
  ui_str.ui_test_0.info_no_found_module = "未检测到模组";
  ui_str.ui_test_0.info_doing_ostest = "开短路测试...";
  ui_str.ui_test_0.prox = "接近";
  ui_str.ui_test_0.als = "环境光";
  ui_str.ui_test_0.mic = "麦克风";
  ui_str.ui_test_0.ostest = "开短路";
  ui_str.ui_test_0.camera = "摄像头";
  ui_str.ui_test_0.camera_found = " 有";
  ui_str.ui_test_0.camera_no_found = " 无";
  ui_str.ui_test_0.spk_p = "喇叭-P";
  ui_str.ui_test_0.spk_n = "喇叭-N";
  ui_str.ui_test_0.ostest_result_normal = "正常";
  ui_str.ui_test_0.ostest_result_exception = "异常";
  ui_str.ui_test_0.key_up_check = "(上键查看)";
  ui_str.ui_test_0.no_ostest = "未测试";
  ui_str.ui_test_0.info_device_exception = "设备异常";

  ui_str.ui_test_1.title = "";
  ui_str.ui_test_1.btn_speaker_play = "喇叭播放";
  ui_str.ui_test_1.btn_menu = ui_str.ui_test_0.btn_menu;
  ui_str.ui_test_1.btn_record_play = ui_str.ui_test_0.btn_record_play;
  ui_str.ui_test_1.info_no_found_module = ui_str.ui_test_0.info_no_found_module;
  ui_str.ui_test_1.info_doing_ostest = ui_str.ui_test_0.info_doing_ostest;
  ui_str.ui_test_1.prox = ui_str.ui_test_0.prox;
  ui_str.ui_test_1.als = ui_str.ui_test_0.als;
  ui_str.ui_test_1.mic = ui_str.ui_test_0.mic;
  ui_str.ui_test_1.ostest = ui_str.ui_test_0.ostest;
  ui_str.ui_test_1.spk_p = ui_str.ui_test_0.spk_p;
  ui_str.ui_test_1.spk_n = ui_str.ui_test_0.spk_n;
  ui_str.ui_test_1.cathc = "喇叭感应";
  ui_str.ui_test_1.yogi = "YOGI";
  ui_str.ui_test_1.hall = "HALL";
  ui_str.ui_test_1.ostest_result_normal = ui_str.ui_test_0.ostest_result_normal;
  ui_str.ui_test_1.ostest_result_exception = ui_str.ui_test_0.ostest_result_exception;
  ui_str.ui_test_1.key_up_check = ui_str.ui_test_0.key_up_check;
  ui_str.ui_test_1.no_ostest = ui_str.ui_test_0.no_ostest;
  ui_str.ui_test_1.info_device_exception = ui_str.ui_test_0.info_device_exception;

  ui_str.ui_test_2.title = "";
  ui_str.ui_test_2.btn_speaker_play = ui_str.ui_test_1.btn_speaker_play;
  ui_str.ui_test_2.btn_menu = ui_str.ui_test_1.btn_menu;
  ui_str.ui_test_2.btn_record_play = ui_str.ui_test_1.btn_record_play;
  ui_str.ui_test_2.info_no_found_module = ui_str.ui_test_1.info_no_found_module;
  ui_str.ui_test_2.info_doing_ostest = ui_str.ui_test_1.info_doing_ostest;
  ui_str.ui_test_2.prox = ui_str.ui_test_1.prox;
  ui_str.ui_test_2.als = ui_str.ui_test_1.als;
  ui_str.ui_test_2.mic = ui_str.ui_test_1.mic;
  ui_str.ui_test_2.ostest = ui_str.ui_test_1.ostest;
  ui_str.ui_test_2.spk_p = ui_str.ui_test_1.spk_p;
  ui_str.ui_test_2.spk_n = ui_str.ui_test_1.spk_n;
  ui_str.ui_test_2.cathc = ui_str.ui_test_1.cathc;
  ui_str.ui_test_2.yogi = ui_str.ui_test_1.yogi;
  ui_str.ui_test_2.ostest_result_normal = ui_str.ui_test_1.ostest_result_normal;
  ui_str.ui_test_2.ostest_result_exception = ui_str.ui_test_1.ostest_result_exception;
  ui_str.ui_test_2.key_up_check = ui_str.ui_test_1.key_up_check;
  ui_str.ui_test_2.no_ostest = ui_str.ui_test_1.no_ostest;
  ui_str.ui_test_2.info_device_exception = ui_str.ui_test_1.info_device_exception;

  ui_str.ui_test_3.title = "";
  ui_str.ui_test_3.btn_speaker_play = ui_str.ui_test_2.btn_speaker_play;
  ui_str.ui_test_3.btn_menu = ui_str.ui_test_2.btn_menu;
  ui_str.ui_test_3.btn_record_play = ui_str.ui_test_2.btn_record_play;
  ui_str.ui_test_3.info_no_found_module = ui_str.ui_test_2.info_no_found_module;
  ui_str.ui_test_3.info_doing_ostest = ui_str.ui_test_2.info_doing_ostest;
  ui_str.ui_test_3.prox = ui_str.ui_test_2.prox;
  ui_str.ui_test_3.als = ui_str.ui_test_2.als;
  ui_str.ui_test_3.mic = ui_str.ui_test_2.mic;
  ui_str.ui_test_3.ostest = ui_str.ui_test_2.ostest;
  ui_str.ui_test_3.spk_p = ui_str.ui_test_2.spk_p;
  ui_str.ui_test_3.spk_n = ui_str.ui_test_2.spk_n;
  ui_str.ui_test_3.cathc = ui_str.ui_test_2.cathc;
  ui_str.ui_test_3.yogi = ui_str.ui_test_2.yogi;
  ui_str.ui_test_3.compass = "指南针";
  ui_str.ui_test_3.ostest_result_normal = ui_str.ui_test_2.ostest_result_normal;
  ui_str.ui_test_3.ostest_result_exception = ui_str.ui_test_2.ostest_result_exception;
  ui_str.ui_test_3.key_up_check = ui_str.ui_test_2.key_up_check;
  ui_str.ui_test_3.no_ostest = ui_str.ui_test_2.no_ostest;
  ui_str.ui_test_3.info_device_exception = ui_str.ui_test_2.info_device_exception;

  ui_str.ui_test_4.title = "";
  ui_str.ui_test_4.btn_menu = ui_str.ui_test_3.btn_menu;
  ui_str.ui_test_4.btn_record_play = ui_str.ui_test_3.btn_record_play;
  ui_str.ui_test_4.info_no_found_module = ui_str.ui_test_3.info_no_found_module;
  ui_str.ui_test_4.info_doing_ostest = ui_str.ui_test_3.info_doing_ostest;
  ui_str.ui_test_4.mic1 = "麦克风1";
  ui_str.ui_test_4.mic1_found = "有";
  ui_str.ui_test_4.mic1_no_found = "无";
  ui_str.ui_test_4.mic = ui_str.ui_test_3.mic;
  ui_str.ui_test_4.ostest = ui_str.ui_test_3.ostest;
  ui_str.ui_test_4.ostest_result_normal = ui_str.ui_test_3.ostest_result_normal;
  ui_str.ui_test_4.ostest_result_oc = "开路";
  ui_str.ui_test_4.ostest_result_sc = "短路";
  ui_str.ui_test_4.ostest_result_total = "总数: ";
  ui_str.ui_test_4.info_device_exception = ui_str.ui_test_3.info_device_exception;
}

static void language_en_config(void)
{
  ui_str.name = "English";

  ui_str.ui_lanuch.update_resource = "Update Resource ...";

  ui_str.ui_charge.charging = "charging...";
  ui_str.ui_charge.full = "fully charged";
  ui_str.ui_charge.discharging = "discharging";

  ui_str.ui_home.title = "Config";
  ui_str.ui_home.btn_update = "Update";
  ui_str.ui_home.btn_start = "Start";
  ui_str.ui_home.btn_menu = "Menu";
  ui_str.ui_home.name = "Name";
  ui_str.ui_home.version = "Version";
  ui_str.ui_home.description = "Description";
  ui_str.ui_home.waiting = "Waiting";

  ui_str.ui_config_update.title = "Update Config";
  ui_str.ui_config_update.btn_back = "Back";
  ui_str.ui_config_update.btn_update = "Update";
  ui_str.ui_config_update.name = "Name";
  ui_str.ui_config_update.version = "Version";
  ui_str.ui_config_update.description = "Description";
  ui_str.ui_config_update.now_config = "Current Config";
  ui_str.ui_config_update.new_config = "New Config";
  ui_str.ui_config_update.updating = "Updating...";
  ui_str.ui_config_update.update_finished = "Update Finished";
  ui_str.ui_config_update.update_failed = "Update Failed";

  ui_str.ui_ostest_result.title = "OSTest Results";
  ui_str.ui_ostest_result.btn_back = "Back";
  ui_str.ui_ostest_result.btn_exp_data = "Export";
  ui_str.ui_ostest_result.btn_learn = "Make";
  ui_str.ui_ostest_result.btn_no = "No";
  ui_str.ui_ostest_result.btn_yes = "Yes";
  ui_str.ui_ostest_result.exporting = "Exporting...";
  ui_str.ui_ostest_result.export_finished = "Export Finished";
  ui_str.ui_ostest_result.export_failed = "Export Failed";
  ui_str.ui_ostest_result.export_data_to_storage = "Export Data To Storage?";
  ui_str.ui_ostest_result.learning = "Learning...";
  ui_str.ui_ostest_result.learn_finished = "Finished";
  ui_str.ui_ostest_result.learn_failed = "Failed";
  ui_str.ui_ostest_result.learn_and_write_to_flash = "Learn and Write To Small Board?";

  ui_str.ui_nonsupport.title = "Error";
  ui_str.ui_nonsupport.info = "Nonsupport!";

  ui_str.ui_menu.title = "Menu";
  ui_str.ui_menu.btn_back = "Back";
  ui_str.ui_menu.btn_confirm = "Confirm";
  ui_str.ui_menu.brightness = "Brightness";
  ui_str.ui_menu.volume = "System Volume";
  ui_str.ui_menu.standby = "Standby";
  ui_str.ui_menu.u_disk_mode = "U Disk Mode";
  ui_str.ui_menu.os_test = "OS Test";
  ui_str.ui_menu.clone_app = "Clone";
  ui_str.ui_menu.language = "Language";
  ui_str.ui_menu.about = "About";

  ui_str.ui_m_brightness.title = "Brightness";
  ui_str.ui_m_brightness.btn_back = "Back";
  ui_str.ui_m_brightness.btn_save = "Save";

  ui_str.ui_m_volume.title = "System Volume";
  ui_str.ui_m_volume.btn_back = "Back";
  ui_str.ui_m_volume.btn_play = "Play";
  ui_str.ui_m_volume.btn_save = "Save";

  ui_str.ui_m_standby.title = "Standby";
  ui_str.ui_m_standby.btn_back = "Back";
  ui_str.ui_m_standby.btn_save = "Save";

  ui_str.ui_m_u_disk_mode.title = "U Disk Mode";
  ui_str.ui_m_u_disk_mode.btn_back = "Back";
  ui_str.ui_m_u_disk_mode.info = "Use the USB port to copy files to built-in storage\n\n\nTo exit, hold down the back key\n";

  ui_str.ui_m_ostest.title = "OS Test";
  ui_str.ui_m_ostest.btn_back = "Back";
  ui_str.ui_m_ostest.btn_edit = "Edit";
  ui_str.ui_m_ostest.function = "Function";
  ui_str.ui_m_ostest.function_enable = "Enable";
  ui_str.ui_m_ostest.function_disable = "Disable";
  ui_str.ui_m_ostest.max_error = "Max Error";

  ui_str.ui_m_clone.title = "Clone";
  ui_str.ui_m_clone.btn_back = "Back";
  ui_str.ui_m_clone.btn_start = "Start";
  ui_str.ui_m_clone.software_version = "Software";
  ui_str.ui_m_clone.device_disconnect = "Device Disconnected";
  ui_str.ui_m_clone.connecting = "Connecting...";
  ui_str.ui_m_clone.connected = "Connected";
  ui_str.ui_m_clone.error = "Error";
  ui_str.ui_m_clone.clone_failed = "Clone Failed";
  ui_str.ui_m_clone.clone_finished = "Clone Finished";
  ui_str.ui_m_clone.start_clone = "Start Clone";

  ui_str.ui_m_language.title = "Language";
  ui_str.ui_m_language.btn_back = "Back";
  ui_str.ui_m_language.btn_select = "Select";
  ui_str.ui_m_language.btn_save = "Save";

  ui_str.ui_m_about.title = "About";
  ui_str.ui_m_about.btn_back = "Back";
  ui_str.ui_m_about.app_version = "APP Version";
  ui_str.ui_m_about.bootloader_version = "BL Version ";

  ui_str.ui_m_ostest_edit.btn_cancel = "Back";
  ui_str.ui_m_ostest_edit.btn_confirm = "OK";

  ui_str.ui_test_0.title = "";
  ui_str.ui_test_0.btn_menu = "Menu";
  ui_str.ui_test_0.btn_record_play = "Rec/Play";
  ui_str.ui_test_0.info_no_found_module = "No Found Module";
  ui_str.ui_test_0.info_doing_ostest = "OS Test...";
  ui_str.ui_test_0.prox = "PROX";
  ui_str.ui_test_0.als = "ALS";
  ui_str.ui_test_0.mic = "MIC";
  ui_str.ui_test_0.ostest = "OSTEST";
  ui_str.ui_test_0.camera = "CAM";
  ui_str.ui_test_0.camera_found = " Have";
  ui_str.ui_test_0.camera_no_found = " None";
  ui_str.ui_test_0.spk_p = "SPK-P";
  ui_str.ui_test_0.spk_n = "SPK-N";
  ui_str.ui_test_0.ostest_result_normal = "Normal";
  ui_str.ui_test_0.ostest_result_exception = "Exception";
  ui_str.ui_test_0.key_up_check = "(KEY-UP Check)";
  ui_str.ui_test_0.no_ostest = "No Test";
  ui_str.ui_test_0.info_device_exception = "Device Exception";

  ui_str.ui_test_1.title = "";
  ui_str.ui_test_1.btn_speaker_play = "SPK-Play";
  ui_str.ui_test_1.btn_menu = ui_str.ui_test_0.btn_menu;
  ui_str.ui_test_1.btn_record_play = ui_str.ui_test_0.btn_record_play;
  ui_str.ui_test_1.info_no_found_module = ui_str.ui_test_0.info_no_found_module;
  ui_str.ui_test_1.info_doing_ostest = ui_str.ui_test_0.info_doing_ostest;
  ui_str.ui_test_1.prox = ui_str.ui_test_0.prox;
  ui_str.ui_test_1.als = ui_str.ui_test_0.als;
  ui_str.ui_test_1.mic = ui_str.ui_test_0.mic;
  ui_str.ui_test_1.ostest = ui_str.ui_test_0.ostest;
  ui_str.ui_test_1.spk_p = ui_str.ui_test_0.spk_p;
  ui_str.ui_test_1.spk_n = ui_str.ui_test_0.spk_n;
  ui_str.ui_test_1.cathc = "CATHC";
  ui_str.ui_test_1.yogi = "YOGI";
  ui_str.ui_test_1.hall = "HALL";
  ui_str.ui_test_1.ostest_result_normal = ui_str.ui_test_0.ostest_result_normal;
  ui_str.ui_test_1.ostest_result_exception = ui_str.ui_test_0.ostest_result_exception;
  ui_str.ui_test_1.key_up_check = ui_str.ui_test_0.key_up_check;
  ui_str.ui_test_1.no_ostest = ui_str.ui_test_0.no_ostest;
  ui_str.ui_test_1.info_device_exception = ui_str.ui_test_0.info_device_exception;

  ui_str.ui_test_2.title = "";
  ui_str.ui_test_2.btn_speaker_play = ui_str.ui_test_1.btn_speaker_play;
  ui_str.ui_test_2.btn_menu = ui_str.ui_test_1.btn_menu;
  ui_str.ui_test_2.btn_record_play = ui_str.ui_test_1.btn_record_play;
  ui_str.ui_test_2.info_no_found_module = ui_str.ui_test_1.info_no_found_module;
  ui_str.ui_test_2.info_doing_ostest = ui_str.ui_test_1.info_doing_ostest;
  ui_str.ui_test_2.prox = ui_str.ui_test_1.prox;
  ui_str.ui_test_2.als = ui_str.ui_test_1.als;
  ui_str.ui_test_2.mic = ui_str.ui_test_1.mic;
  ui_str.ui_test_2.ostest = ui_str.ui_test_1.ostest;
  ui_str.ui_test_2.spk_p = ui_str.ui_test_1.spk_p;
  ui_str.ui_test_2.spk_n = ui_str.ui_test_1.spk_n;
  ui_str.ui_test_2.cathc = ui_str.ui_test_1.cathc;
  ui_str.ui_test_2.yogi = ui_str.ui_test_1.yogi;
  ui_str.ui_test_2.ostest_result_normal = ui_str.ui_test_1.ostest_result_normal;
  ui_str.ui_test_2.ostest_result_exception = ui_str.ui_test_1.ostest_result_exception;
  ui_str.ui_test_2.key_up_check = ui_str.ui_test_1.key_up_check;
  ui_str.ui_test_2.no_ostest = ui_str.ui_test_1.no_ostest;
  ui_str.ui_test_2.info_device_exception = ui_str.ui_test_1.info_device_exception;

  ui_str.ui_test_3.title = "";
  ui_str.ui_test_3.btn_speaker_play = ui_str.ui_test_2.btn_speaker_play;
  ui_str.ui_test_3.btn_menu = ui_str.ui_test_2.btn_menu;
  ui_str.ui_test_3.btn_record_play = ui_str.ui_test_2.btn_record_play;
  ui_str.ui_test_3.info_no_found_module = ui_str.ui_test_2.info_no_found_module;
  ui_str.ui_test_3.info_doing_ostest = ui_str.ui_test_2.info_doing_ostest;
  ui_str.ui_test_3.prox = ui_str.ui_test_2.prox;
  ui_str.ui_test_3.als = ui_str.ui_test_2.als;
  ui_str.ui_test_3.mic = ui_str.ui_test_2.mic;
  ui_str.ui_test_3.ostest = ui_str.ui_test_2.ostest;
  ui_str.ui_test_3.spk_p = ui_str.ui_test_2.spk_p;
  ui_str.ui_test_3.spk_n = ui_str.ui_test_2.spk_n;
  ui_str.ui_test_3.cathc = ui_str.ui_test_2.cathc;
  ui_str.ui_test_3.yogi = ui_str.ui_test_2.yogi;
  ui_str.ui_test_3.compass = "COMPASS";
  ui_str.ui_test_3.ostest_result_normal = ui_str.ui_test_2.ostest_result_normal;
  ui_str.ui_test_3.ostest_result_exception = ui_str.ui_test_2.ostest_result_exception;
  ui_str.ui_test_3.key_up_check = ui_str.ui_test_2.key_up_check;
  ui_str.ui_test_3.no_ostest = ui_str.ui_test_2.no_ostest;
  ui_str.ui_test_3.info_device_exception = ui_str.ui_test_2.info_device_exception;

  ui_str.ui_test_4.title = "";
  ui_str.ui_test_4.btn_menu = ui_str.ui_test_3.btn_menu;
  ui_str.ui_test_4.btn_record_play = ui_str.ui_test_3.btn_record_play;
  ui_str.ui_test_4.info_no_found_module = ui_str.ui_test_3.info_no_found_module;
  ui_str.ui_test_4.info_doing_ostest = ui_str.ui_test_3.info_doing_ostest;
  ui_str.ui_test_4.mic1 = "MIC1";
  ui_str.ui_test_4.mic1_found = "Have";
  ui_str.ui_test_4.mic1_no_found = "None";
  ui_str.ui_test_4.mic = ui_str.ui_test_3.mic;
  ui_str.ui_test_4.ostest = ui_str.ui_test_3.mic;
  ui_str.ui_test_4.ostest_result_normal = ui_str.ui_test_3.ostest_result_normal;
  ui_str.ui_test_4.ostest_result_oc = "OC";
  ui_str.ui_test_4.ostest_result_sc = "SC";
  ui_str.ui_test_4.ostest_result_total = "";
  ui_str.ui_test_4.info_device_exception = ui_str.ui_test_3.info_device_exception;
}

static const char* ml_get_str(const ml_str_file_st* ml_file, ml_id_et id)
{
  const char* str = 0;
  uint8_t* pdata, * end;

  if(ml_file)
  {
    pdata = (uint8_t*)ml_file->data;
    end = pdata + ml_file->size;

    for(;pdata < end;)
    {
      uint16_t tmp_id;
      const char* tmp_str;
      __IO uint8_t tmp_l, tmp_h;

      tmp_l = pdata[0];
      tmp_h = pdata[1];
      tmp_id = tmp_l;
      tmp_id |= (tmp_h << 8);
      tmp_str = (const char*)(pdata + sizeof(ml_id_et));//((ml_str_file_dta_st *)pdata)->str;

      if(tmp_id == id)
      {
        str = tmp_str;
        break;
      }

      pdata += strlen(tmp_str) + 1 + sizeof(ml_id_et);
    }
  }

  return str;
}

/**
 * @brief 使用语言文件
 *
 * @param path 文件完整路径
 */
void ml_set(const char* path)
{
  FRESULT res;
  FIL fil;
  UINT br;
  static ml_str_file_st* ml_file = 0;

  //设置默认语言
  {
    const char** p = (const char**)&ui_str;
    for(uint32_t i = 0;i < sizeof(ui_str) / sizeof(p[0]);i++)
    {
      p[i] = "";
    }
    language_en_config();
  }

  if(ml_file)
  {
    vPortFree(ml_file);
    ml_file = 0;
  }

  //加载文件到内存
  if(path)
  {
    ML_DBG("- ML use %s.\n", path);

    ML_DBG("-- open file %s ... ", path);
    res = f_open(&fil, path, FA_READ);
    if(res == FR_OK)
    {
      ML_DBG("OK.\n");
      ML_DBG("-- malloc(%d) ... ", f_size(&fil));
      ml_file = pvPortMalloc(f_size(&fil));
      if(ml_file == 0)
      {
        ML_DBG("Fail.\n");
        f_close(&fil);
      }
      else
      {
        ML_DBG("OK. addr:0x%08X.\n", ml_file);
        ML_DBG("-- read file ... ");
        res = f_read(&fil, ml_file, f_size(&fil), &br);
        if(res != FR_OK)
        {
          ML_DBG("Fail.\n");
          ML_DBG("-- free memory.\n");
          vPortFree(ml_file);
          ml_file = 0;
        }
        else
        {
          ML_DBG("OK.\n");
        }

        ML_DBG("-- close file.\n");
        f_close(&fil);
      }
    }
    else
    {
      ML_DBG("Fail.\n");
    }
  }

  if(ml_file)
  {
    //解析文件
    ML_DBG("-- content:\n");
    for(uint32_t i = 0;i < sizeof(ml_str_id) / sizeof(ml_str_id_st);i++)
    {
      const char* str = ml_get_str(ml_file, ml_str_id[i].id);
      if(str)
      {
        *ml_str_id[i].p = str;
        ML_DBG("--id:%04X str:%s\n", ml_str_id[i].id, ml_get_str(ml_file, ml_str_id[i].id));
      }
    }
  }
}

/**
 * @brief 检查语言文件
 *
 * @param file 文件
 * @return int （0：正确）（1：错误）
 */
int ml_check(ml_str_file_st* file)
{
  if(strcmp(file->head, ".ml") == 0)
    return 0;
  else
    return 1;
}

/**
 * @brief 获取语言包文件的语言名称
 *
 * @param path 文件完整路径
 * @param buf 填充语言名称的缓存
 * @param max_len 缓存最大长度
 * @return int 错误码
 */
int ml_get_name(const char* path, char* buf, uint32_t max_len)
{
  FRESULT res;
  FIL* fil;
  UINT br;
  ml_str_file_st* pdata;
  int err;

  fil = pvPortMalloc(sizeof(FIL));
  if(fil == 0)
  {
    err = -1;
    goto __exit;
  }

  memset(fil, 0, sizeof(FIL));

  res = f_open(fil, path, FA_READ);
  if(res == FR_OK)
  {
    pdata = pvPortMalloc(f_size(fil));
    if(pdata)
    {
      res = f_read(fil, pdata, f_size(fil), &br);
      if(res == FR_OK)
      {
        if(ml_check(pdata) == 0)
        {
          snprintf(buf, max_len, "%s", ml_get_str(pdata, ML_NAME));
          err = 0;
        }
        else
        {
          err = -5;
        }
      }
      else
      {
        err = -4;
      }
      vPortFree(pdata);
    }
    else
    {
      err = -3;
    }
    f_close(fil);
  }
  else
  {
    err = 2;
  }

  vPortFree(fil);

__exit:
  return err;
}
