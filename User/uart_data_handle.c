#include "uart_data_handle.h"
#include "my_config.h"
#include "uart0.h"

#include <stdio.h> // printf

static volatile u8 cmd_buff[10] = {0};    // 存放接收到的一条指令
static volatile u8 cur_cmd_buff_len = 0;  // 指示当前接收到的指令的索引（之后会在程序中更新，不用清零）
static volatile u8 dest_cmd_buff_len = 0; // 存放最终要接收的指令长度（之后会在程序中更新，不用清零）

static volatile u8 timeout_enable = 0; // 超时计数使能
static volatile u16 timeout_cnt = 0;   // 超时计数

// 由定时中断调用，累计超时计数
void uart_data_recv_timeout_add(void)
{
    if (timeout_enable)
    {
        timeout_cnt += 10; // 有计数溢出的风险，要注意在溢出前进行处理
    }
}

void uart_data_handle(void)
{
    static volatile u8 uart_data_handle_status = UART_DATA_HANDLE_STATUS_IDLE; // 状态机

    u8 recv_byte;
    u8 check_sum = 0;        // 存放计算之后的校验和
    u8 i;                    // 循环计数值
    u8 is_recv_complete = 0; // 是否接收到完整的一帧指令

    if (0 == uart0_rxbuffer_get_count())
    {
        // 未收到数据，累加超时计时
        if (timeout_cnt >= UART_DATA_HANDLE_TIMEOUT)
        {
            // 接收超时
            timeout_cnt = 0;
            timeout_enable = 0;                                     // 不使能超时计数
            uart_data_handle_status = UART_DATA_HANDLE_STATUS_IDLE; // 重新开始接收

            // 打印超时之后，缓冲区内的数据
            printf("=================================>\n");
            printf("uart recv timeout\n");
            for (i = 0; i < ARRAY_SIZE(cmd_buff); i++)
            {
                printf("%02x", (u16)cmd_buff[i]);
            }
            printf("=================================^\n");
        }

        return; // 串口缓冲区的数据为空，直接返回
    }

    while (1) // 一次性把缓冲区中的数据读出来
    {
        if (uart0_rxbuffer_get_count() == 0 || is_recv_complete) // 退出条件
        {
            if (is_recv_complete)
            {
                is_recv_complete = 0;
            }

            break;
        }

        timeout_enable = 1; // 使能超时计数
        timeout_cnt = 0;
        recv_byte = uart0_rxbuffer_get_byte();

        switch (uart_data_handle_status)
        {
        case UART_DATA_HANDLE_STATUS_FORMAT_HEAD0:
            if (UART_DATA_HANDLE_FORMAT_HEAD0 == recv_byte)
            {
                cmd_buff[0] = recv_byte;
                uart_data_handle_status = UART_DATA_HANDLE_STATUS_FORMAT_HEAD1;
            }
            break;
        case UART_DATA_HANDLE_STATUS_FORMAT_HEAD1:
            if (UART_DATA_HANDLE_FORMAT_HEAD1 == recv_byte)
            {
                cmd_buff[1] = recv_byte;
                uart_data_handle_status = UART_DATA_HANDLE_STATUS_FORMAT_FIX_VAL0;
            }
            else
            {
                // 如果数据有误，重新开始接收
                uart_data_handle_status = UART_DATA_HANDLE_STATUS_FORMAT_HEAD0;
            }
            break;
        case UART_DATA_HANDLE_STATUS_FORMAT_FIX_VAL0:
            if (UART_DATA_HANDLE_FORMAT_FIX_VAL0 == recv_byte)
            {
                cmd_buff[2] = recv_byte;
                uart_data_handle_status = UART_DATA_HANDLE_STATUS_FORMAT_CMD;
            }
            else
            {
                // 如果数据有误，重新开始接收
                uart_data_handle_status = UART_DATA_HANDLE_STATUS_FORMAT_HEAD0;
            }

            break;
        case UART_DATA_HANDLE_STATUS_FORMAT_CMD:
            cmd_buff[3] = recv_byte;
            uart_data_handle_status = UART_DATA_HANDLE_STATUS_FORMAT_DATA;
            break;
        }
    }

    if (UART_DATA_HANDLE_STATUS_FORMAT_TAIL != uart_data_handle_status)
    {
        return; // 未接收完数据，不进入下面的处理操作，函数直接返回
    }

    // 打印接收到的一帧数据
    for (i = 0; i < dest_cmd_buff_len; i++)
    {
        printf("0x%02x ", (u16)cmd_buff[i]);
    }
    printf("\n");

    switch (cmd_buff[2])
    {

    default:
        break;
    }

    // 处理完成后，重新接收数据
    timeout_cnt = 0;
    timeout_enable = 0; // 不使能超时计数
    uart_data_handle_status = UART_DATA_HANDLE_STATUS_IDLE;

#if 0
    static volatile u8 cmd_buff[10] = {0};    // 存放接收到的一条指令
    static volatile u8 cur_cmd_buff_len = 0;  // 指示当前接收到的指令的索引（之后会在程序中更新，不用清零）
    static volatile u8 dest_cmd_buff_len = 0; // 存放最终要接收的指令长度（之后会在程序中更新，不用清零）

    static volatile u8 timeout_enable = 0; // 超时计数使能
    static volatile u32 timeout_cnt = 0;   // 超时计数（基于系统时基，运行时值不为0）

    static volatile u8 uart_data_handle_status = UART_DATA_HANDLE_STATUS_IDLE; // 状态机

    u8 recv_byte;
    u8 check_sum = 0;        // 存放计算之后的校验和
    u8 i;                    // 循环计数值
    u8 is_recv_complete = 0; // 是否接收到完整的一帧指令

    // 接收超时处理：
    if (0 == uart_rxbuffer_get_count())
    {
        if (timeout_enable &&
            tick_check_expire(timeout_cnt, UART_DATA_HANDLE_TIMEOUT))
        {
            // 接收超时
            // timeout_cnt = 0;
            // timeout_cnt = tick_get();
            timeout_enable = 0;                                     // 不使能超时计数
            uart_data_handle_status = UART_DATA_HANDLE_STATUS_IDLE; // 重新开始接收

            // 超时之后，打印缓冲区内的数据
            my_printf("=================================>\n");
            my_printf("uart recv timeout\n");
            for (i = 0; i < ARRAY_SIZE(cmd_buff); i++)
            {
                my_printf("%02x ", (u16)cmd_buff[i]);
            }
            my_printf("=================================^\n");
        }

        return; // 串口缓冲区的数据为空，直接返回
    }

    while (1) // 有时候该函数会100~200ms才调用一次，这里一次性把缓冲区中的数据读出来
    {
        if (uart_rxbuffer_get_count() == 0 || is_recv_complete) // 退出条件
        {
            if (is_recv_complete)
            {
                is_recv_complete = 0;
            }

            break;
        }

        timeout_enable = 1;       // 使能超时计数
        timeout_cnt = tick_get(); // 更新超时计数的时基
        recv_byte = uart_rxbuffer_get_byte();

        switch (uart_data_handle_status)
        {
        case UART_DATA_HANDLE_STATUS_IDLE:
            if (UART_DATA_HANDLE_FORMAT_HEAD == recv_byte)
            {
                cmd_buff[0] = recv_byte;
                cur_cmd_buff_len = 1;
                uart_data_handle_status = UART_DATA_HANDLE_STATUS_FORMAT_HEAD;
            }
            else
            {
                // 不是格式头，重新开始接收，关掉超时计数
                timeout_enable = 0;
            }
            break;
            // ===============================================================
        case UART_DATA_HANDLE_STATUS_FORMAT_HEAD:
            cmd_buff[cur_cmd_buff_len++] = recv_byte;
            dest_cmd_buff_len = recv_byte;                         // 存放要接收的数据长度
            uart_data_handle_status = UART_DATA_HANDLE_STATUS_LEN; // 表示接收到了数据帧长度
            // my_printf("len == %bu\n", dest_cmd_buff_len);
            break;
            // ===============================================================
        case UART_DATA_HANDLE_STATUS_LEN:
            cmd_buff[cur_cmd_buff_len++] = recv_byte;
            if (cur_cmd_buff_len >= dest_cmd_buff_len) // 如果接收完所有的数据
            {
                for (i = 0; i < dest_cmd_buff_len - 1; i++)
                {
                    check_sum += cmd_buff[i];
                }

                if (check_sum != cmd_buff[dest_cmd_buff_len - 1])
                {
                    // 校验和错误
                    my_printf("=================================>\n");
                    my_printf("check sum error\n");
                    for (i = 0; i < ARRAY_SIZE(cmd_buff); i++)
                    {
                        my_printf("%02x ", (u16)cmd_buff[i]);
                    }
                    my_printf("=================================^\n");

                    // timeout_cnt = 0;
                    timeout_enable = 0;                                     // 不使能超时计数
                    uart_data_handle_status = UART_DATA_HANDLE_STATUS_IDLE; // 重新接收数据
                }
                else
                {
                    // 校验和正确
                    my_printf("check sum ok\n");
                    uart_data_handle_status = UART_DATA_HANDLE_STATUS_END;
                    is_recv_complete = 1;
                }
            }
            break;
        // ===============================================================
        default:

            break;
        }
    }

    if (UART_DATA_HANDLE_STATUS_END != uart_data_handle_status)
    {
        return; // 未接收完数据，不进入下面的处理操作，函数直接返回
    }

    // 打印接收到的一帧数据
    // for (i = 0; i < dest_cmd_buff_len; i++)
    // {
    //     my_printf("0x%02x ", (u16)cmd_buff[i]);
    // }
    // my_printf("\n");

    // 处理一帧数据：
    switch (cmd_buff[2])
    {
    }

    // 处理完成后，重新接收数据
    // timeout_cnt = 0;
    timeout_enable = 0; // 不使能超时计数
    uart_data_handle_status = UART_DATA_HANDLE_STATUS_IDLE;

#endif
}
