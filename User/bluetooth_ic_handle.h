#ifndef __BLUETOOTH_IC_HANDLE__
#define __BLUETOOTH_IC_HANDLE__

#include "include.h"

// 定义蓝牙ic状态
enum
{
    BLUETOOTH_IC_STATUS_IDLE, // 蓝牙不工作
    BLUETOOTH_IC_STATUS_PLAYING_MUSIC, // 正在播放音乐
    BLUETOOTH_IC_STATUS_PAUSE_MUSIC,   // 暂停播放音乐
};
typedef u8 bluetooth_ic_status_t;

typedef struct
{
    bluetooth_ic_status_t status;
} bluetooth_ic_handler_t;

#endif