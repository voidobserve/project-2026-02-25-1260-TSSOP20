#ifndef __BLUETOOTH_IC_HANDLE__
#define __BLUETOOTH_IC_HANDLE__

#include "include.h"

#define BLE_IC_POWER_KEY_PIN P23

// 定义蓝牙ic状态
enum
{
    // BLUETOOTH_IC_STATUS_IDLE,          // 蓝牙不工作
    BLUETOOTH_IC_STATUS_PLAYING_MUSIC, // 正在播放音乐
    BLUETOOTH_IC_STATUS_PAUSE_MUSIC,   // 暂停播放音乐
    BLUETOOTH_IC_STATUS_CONNECTED,     // 蓝牙连接成功
    BLUETOOTH_IC_STATUS_DISCONNECT,    // 蓝牙断开连接

    // BLUETOOTH_IC_STATUS_AMP_OFF,       // 蓝牙关机时已经关闭功放（关机时，单片机先通知蓝牙关闭，蓝牙会mute功放，返回此指令后单片机再关电）
};
typedef u8 bluetooth_ic_status_t;

typedef struct
{
    // bluetooth_ic_status_t status;
    u8 music_status; // 音乐播放状态
    // u8 connect_status; // 蓝牙连接状态
    // u8 is_amp_off; // 功放是否关闭
    u8 is_working; // 蓝牙ic是否工作
} bluetooth_ic_t;

extern volatile bluetooth_ic_t ble_ic;

void bluetooth_ic_handle_init(void);

void ble_ic_enable(void);
void ble_ic_disable_pre(void);
void ble_ic_disable_post(void);

#endif