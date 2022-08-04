#ifndef _CONFIG_H
#define _CONFIG_H

// 设备名称
#define DEVICE_NAME "XP模组测试机"

// 软件版本
#define SOFTWARE_VERSION "0.1.1"

// <<< Use Configuration Wizard in Context Menu >>>

//<o>  Min Logo Time (in ms) <0x0-0xFFFFFFFF>
// 开机logo至少显示时间,单位毫秒
#define BOOT_LOGO_TIME_MIN (1000)

//<o> Show Version On Booting <0=>No <1=>Yes
// 开机过程在屏幕下方显示软件版本
#define BOOT_SHOW_VERSION (1)

//<s> Wav File Of Board loaded
// 发现小板被扣上时播放的声音文件
#define WAV_FILE_BOARD_LOADED "0:/audio/load.wav"

//<s> Wav File Of Warning
// 警报声音文件
#define WAV_FILE_WARNING "0:/audio/warning.wav"

// <<< end of configuration section >>>

#endif
