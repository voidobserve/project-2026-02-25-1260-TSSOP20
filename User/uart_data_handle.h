#ifndef __UART_DATA_HANDLE_H__
#define __UART_DATA_HANDLE_H__

#include "include.h"

#define UART_DATA_HANDLE_TIMEOUT ((u16)2000) // 接收数据的超时时间，单位：ms
#define UART_MAX_FRAME_LENGTH 10             // 最大帧长度

/*
    一帧数据构成：
    head0, head1, 0x00, cmd, data0, 0x00, check sum, tail
    check sum == head0 + head1 + 0x00 + cmd + data0
*/
#define UART_DATA_HANDLE_FORMAT_HEAD0 0xA5    // 数据格式头
#define UART_DATA_HANDLE_FORMAT_HEAD1 0xFA    // 数据格式头
#define UART_DATA_HANDLE_FORMAT_FIX_VAL0 0x00 // 固定值0
#define UART_DATA_HANDLE_FORMAT_FIX_VAL1 0x00 // 固定值1
#define UART_DATA_HANDLE_FORMAT_TAIL 0xFB     // 数据尾部

// 串口接收器接收的串口控制命令
// USER_TO_DO
enum
{
    UART_RECV_CMD_,
};
typedef u8 uart_recv_cmd_t;

// 串口发送的串口控制命令
enum
{
    UART_SEND_CMD_BLE_OPEN = 0x0B,           // 打开蓝牙
    UART_SEND_CMD_BLE_CLOSE = 0x0D,          // 关闭蓝牙
    UART_SEND_CMD_MUSIC_PLAY = 0x01,         // 播放音乐
    UART_SEND_CMD_MUSIC_PAUSE = 0x02,        // 暂停播放
    UART_SEND_CMD_MUSIC_PREV = 0x03,         // 上一曲
    UART_SEND_CMD_MUSIC_NEXT = 0x04,         // 下一曲
    UART_SEND_CMD_VOLUME_ADD = 0x07,         // 音量加
    UART_SEND_CMD_VOLUME_SUB = 0x08,         // 音量减
    UART_SEND_CMD_LOW_POWER_WARNING = 0x09,  // 低电量提示
    UART_SEND_CMD_LOW_POWER_SHUTDOWN = 0x0A, // 低电量关机
};
typedef u8 uart_send_cmd_t;

// UART接收器状态枚举
enum
{
    UART_DATA_HANDLE_STATUS_IDLE = 0,
    UART_DATA_HANDLE_STATUS_FORMAT_HEAD0,     // 格式头0
    UART_DATA_HANDLE_STATUS_FORMAT_HEAD1,     // 格式头1
    UART_DATA_HANDLE_STATUS_FORMAT_FIX_VAL0,  // 固定值0
    UART_DATA_HANDLE_STATUS_FORMAT_CMD,       // 命令字节
    UART_DATA_HANDLE_STATUS_FORMAT_DATA,      // 数据字节
    UART_DATA_HANDLE_STATUS_FORMAT_FIX_VAL1,  // 固定值1
    UART_DATA_HANDLE_STATUS_FORMAT_CHECK_SUM, // 校验和
    UART_DATA_HANDLE_STATUS_FORMAT_TAIL,      // 帧尾
    // UART_DATA_HANDLE_STATUS_COMPLETE          // 接收完成
};
typedef u8 uart_status_t;

// UART接收器结构体
typedef struct
{
    u8 buffer[UART_MAX_FRAME_LENGTH]; // 接收缓冲区
    u8 index;                         // 当前接收位置
    u8 status;                        // 当前状态
    u8 checksum;                      // 计算的校验和
    u8 timeout_enable;                // 超时使能标志
    u16 timeout_cnt;                  // 超时计数器
} uart_receiver_t;

void uart_receiver_reset(void);
void uart_receiver_timeout_add(void);
u8 uart_receiver_process_checksum(void);
u8 uart_receiver_process_byte(u8 byte);
void uart_receiver_timeout_handler(void);
void uart_data_handle(void);

void uart_data_send_cmd(uart_send_cmd_t cmd);

#endif