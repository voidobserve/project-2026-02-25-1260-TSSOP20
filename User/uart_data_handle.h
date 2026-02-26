#ifndef __UART_DATA_HANDLE_H__
#define __UART_DATA_HANDLE_H__

#include "include.h"

#define UART_DATA_HANDLE_TIMEOUT ((u16)2000) // 接收数据的超时时间，单位：ms

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

enum
{
    UART_DATA_HANDLE_STATUS_IDLE = 0,
    UART_DATA_HANDLE_STATUS_FORMAT_HEAD0,     // 格式头
    UART_DATA_HANDLE_STATUS_FORMAT_HEAD1,     // 格式头 1
    UART_DATA_HANDLE_STATUS_FORMAT_FIX_VAL0,  //
    UART_DATA_HANDLE_STATUS_FORMAT_CMD,       //
    UART_DATA_HANDLE_STATUS_FORMAT_DATA,      //
    UART_DATA_HANDLE_STATUS_FORMAT_FIX_VAL1,  //
    UART_DATA_HANDLE_STATUS_FORMAT_CHECK_SUM, //
    UART_DATA_HANDLE_STATUS_FORMAT_TAIL,      //

    // UART_DATA_HANDLE_STATUS_END,
};

void uart_data_handle(void);

void uart_data_recv_timeout_add(void);

#endif