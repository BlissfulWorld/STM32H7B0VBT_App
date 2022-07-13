#ifndef _CONFIG_H
#define _CONFIG_H

// 设备名称
#define DEVICE_NAME "XP模组测试机"

// 软件版本
#define SOFTWARE_VERSION "0.1.0"

// <<< Use Configuration Wizard in Context Menu >>>

//<o>  Min Logo Time (in ms) <0x0-0xFFFFFFFF>
// 开机logo至少显示时间,单位毫秒
#define BOOT_LOGO_TIME_MIN (1000)

//<e> Make Board License
//生成LICENSE并下载和初始化测试板配置文件
#define MAKE_BOARD_LICENSE 0

//<o>Model <0=>XP7 <1=>XP7P <2=>XP8P
#define MAKE_BOARD_MODEL (1)
//</e>

// <<< end of configuration section >>>

#define XP7  0
#define XP7P 1
#define XP8P 2

#endif
