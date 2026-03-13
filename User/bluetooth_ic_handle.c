#include "bluetooth_ic_handle.h"
#include "user_config.h"
#include "uart0.h"

volatile bluetooth_ic_t ble_ic;

void bluetooth_ic_handle_init(void)
{
    // 连接到蓝牙ic的 power key，蓝牙开机时输出低电平，蓝牙关机之后输出高电平
    P2_MD0 &= ~GPIO_P23_MODE_SEL(0x03);
    P2_MD0 |= GPIO_P23_MODE_SEL(0x01);
    FOUT_S23 = GPIO_FOUT_AF_FUNC;
    P23 = 1; // 默认高电平，不打开蓝牙

    ble_ic.music_status = BLUETOOTH_IC_STATUS_PAUSE_MUSIC;
    // ble_ic.connect_status = BLUETOOTH_IC_STATUS_DISCONNECT;
    // ble_ic.is_amp_off = 0;
    ble_ic.is_working = 0;
}
