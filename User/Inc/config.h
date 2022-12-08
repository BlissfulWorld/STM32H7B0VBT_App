#ifndef _CONFIG_H
#define _CONFIG_H

// 设备名称
#define DEVICE_NAME "XP Test"

// 软件版本
#define SOFTWARE_VERSION "0.6.1"

// <<< Use Configuration Wizard in Context Menu >>>

//<o>  Min Logo Time (units: ms) <0x0-0xFFFFFFFF>
// 开机logo至少显示时间,单位毫秒
#define BOOT_LOGO_TIME_MIN (2000)

//<o> Show Version During Booting <0=>No <1=>Yes
// 开机过程在屏幕下方显示软件版本
#define BOOT_SHOW_VERSION (1)

//<o> Screenshot Function <0=>Disable <1=>Enable
// 截屏功能
#define SCREENSHOT_ENABLE (0)

//<o>  Mic Record Time Depth (units: s) <0x1-0xFFFFFFFF>
// 麦克风录制时间深度(最长可以连续录制多久)，单位秒
#define MIC_RECORD_TIME_DEPTH (5)

//<s> Wav File Of Board loaded
// 发现小板被扣上时播放的声音文件
#define WAV_FILE_BOARD_LOADED "1:/audio/load.wav"

//<s> Wav File Of Warning
// 警报声音文件
#define WAV_FILE_WARNING "1:/audio/warning.wav"

//<s> Path Of New Config File
// 新配置文件
#define CONFIG_FILE_UPDATE_PATH "2:/config.json"

// <<< end of configuration section >>>

#define DISK_SYS_PATH  USERPath
#define DISK_USER_PATH USERPath2
#define DISK_EXT_PATH  USBHPath

#define DISK_SYS_FS  USERFatFS
#define DISK_USER_FS USERFatFS2
#define DISK_EXT_FS  USBHFatFS

#endif
