#include "bluetooth_ic_handle.h"
#include "user_config.h"
#include "user_include.h"
#include "uart0.h"

volatile bluetooth_ic_t ble_ic;

// 蓝牙ic打开
void ble_ic_enable(void)
{
    BLE_IC_POWER_KEY_PIN = 0;
    delay_ms(30); // 等待蓝牙开机完毕
    uart_data_send_cmd(UART_SEND_CMD_BLE_OPEN);
    ble_ic.is_working = 1;
}

// 蓝牙ic关闭 前置操作
void ble_ic_disable_pre(void)
{
    uart_data_send_cmd(UART_SEND_CMD_BLE_CLOSE);
    // 发送完成后，需要在串口接收到蓝牙关闭功放的数据后，再执行关闭蓝牙的操作
}

// 蓝牙ic关闭 后置操作
void ble_ic_disable_post(void)
{
    // 收到蓝牙ic回复的数据后，关闭蓝牙ic
    BLE_IC_POWER_KEY_PIN = 1;
    ble_ic.is_working = 0;
}

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
