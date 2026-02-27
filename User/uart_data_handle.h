#ifndef __UART_DATA_HANDLE_H__
#define __UART_DATA_HANDLE_H__

#include "include.h"

#define UART_DATA_HANDLE_TIMEOUT ((u16)2000) // 接收数据的超时时间，单位：ms
#define UART_MAX_FRAME_LENGTH 10             // 最大帧长度

/*
    一帧数据构成：
    head0,head1,0x00,cmd,data0,0x00,check sum,tail
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
    UART_SEND_CMD_,
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

#endif