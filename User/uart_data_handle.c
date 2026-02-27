#include "uart_data_handle.h"
#include "user_config.h"
#include "uart0.h"
#include <stdio.h>
#include <string.h>

// 接收器
static volatile uart_receiver_t uart_receiver;

// 重置接收器状态
void uart_receiver_reset(void)
{
    memset((void *)&uart_receiver, 0, sizeof(uart_receiver_t));
    uart_receiver.status = UART_DATA_HANDLE_STATUS_IDLE;
}

/**
 * @brief 接收器超时使能时，累计超时时间，在定时器中断中调用
 *
 */
void uart_receiver_timeout_add(void)
{
    if (uart_receiver.timeout_enable)
    {
        uart_receiver.timeout_cnt++;
    }
}

/**
 * @brief 计算校验和。有接收完校验和之后，才调用该函数
 *
 * @return u8 0：校验通过；   1：校验不通过
 */
u8 uart_receiver_process_checksum(void)
{
    u8 i;
    u8 check_sum = 0;
    for (i = 0; i < 5; i++) // 只计算前5个字节的校验和
    {
        check_sum += uart_receiver.buffer[i];
    }

    if (check_sum == uart_receiver.buffer[6])
    {
        return 0;
    }
    else
    {
#if USER_DEBUG_ENABLE
        // 校验和错误
        printf("Checksum error: expected 0x%02x, got 0x%02x\n",
               (u16)check_sum, (u16)uart_receiver.buffer[6]);
#endif

        return 1;
    }
}

/**
 * @brief 接收器处理单个字节
 *
 * @param byte
 * @return u8 0：接收成功
 *            非0：接收失败
 */
u8 uart_receiver_process_byte(u8 byte)
{
    switch (uart_receiver.status)
    {
    case UART_DATA_HANDLE_STATUS_IDLE:
        if (byte == UART_DATA_HANDLE_FORMAT_HEAD0)
        {
            uart_receiver.buffer[uart_receiver.index++] = byte;
            uart_receiver.status = UART_DATA_HANDLE_STATUS_FORMAT_HEAD0;
            return 0;
        }
        else
        {
            return 1;
        }

        break;

    case UART_DATA_HANDLE_STATUS_FORMAT_HEAD0:
        if (byte == UART_DATA_HANDLE_FORMAT_HEAD1)
        {
            uart_receiver.buffer[uart_receiver.index++] = byte;
            uart_receiver.status = UART_DATA_HANDLE_STATUS_FORMAT_HEAD1;
            return 0;
        }
        else
        {
            // 错误
            return 1;
        }
        break;

    case UART_DATA_HANDLE_STATUS_FORMAT_HEAD1:
        if (byte == UART_DATA_HANDLE_FORMAT_FIX_VAL0)
        {
            uart_receiver.buffer[uart_receiver.index++] = byte;
            uart_receiver.status = UART_DATA_HANDLE_STATUS_FORMAT_FIX_VAL0;
            return 0;
        }
        else
        {
            return 1;
        }
        break;

    case UART_DATA_HANDLE_STATUS_FORMAT_FIX_VAL0:
        // CMD字节，任何值都可以接受
        uart_receiver.buffer[uart_receiver.index++] = byte;
        uart_receiver.status = UART_DATA_HANDLE_STATUS_FORMAT_CMD;
        return 0;
        break;

    case UART_DATA_HANDLE_STATUS_FORMAT_CMD:
        // DATA字节，任何值都可以接受
        uart_receiver.buffer[uart_receiver.index++] = byte;
        uart_receiver.status = UART_DATA_HANDLE_STATUS_FORMAT_DATA;
        return 0;
        break;

    case UART_DATA_HANDLE_STATUS_FORMAT_DATA:
        if (byte == UART_DATA_HANDLE_FORMAT_FIX_VAL1)
        {
            uart_receiver.buffer[uart_receiver.index++] = byte;
            uart_receiver.status = UART_DATA_HANDLE_STATUS_FORMAT_FIX_VAL1;
            return 0;
        }
        else
        {
            return 1;
        }

    case UART_DATA_HANDLE_STATUS_FORMAT_FIX_VAL1:
        // 校验和字节
        uart_receiver.buffer[uart_receiver.index++] = byte;
        uart_receiver.status = UART_DATA_HANDLE_STATUS_FORMAT_CHECK_SUM;
        if (uart_receiver_process_checksum())
        {
            // 校验和出错
            return 1;
        }
        else
        {
            return 0;
        }
        break;

    case UART_DATA_HANDLE_STATUS_FORMAT_CHECK_SUM:
        if (byte == UART_DATA_HANDLE_FORMAT_TAIL)
        {
            uart_receiver.buffer[uart_receiver.index++] = byte;
            uart_receiver.status = UART_DATA_HANDLE_STATUS_FORMAT_TAIL;
            return 0;
        }
        else
        {
            return 1;
        }
        break;

    default:
        break;
    }

    return 1;
}

// 超时处理函数
void uart_receiver_timeout_handler(void)
{
    u8 i;
    if (uart_receiver.timeout_cnt >= UART_DATA_HANDLE_TIMEOUT)
    {
#if USER_DEBUG_ENABLE
        printf("UART receive timeout, current status: %d\n", (u16)uart_receiver.status);

        // 打印当前缓冲区内容
        printf("Buffer content: \n");
        for (i = 0; i < uart_receiver.index; i++)
        {
            printf("0x%02x ", (u16)uart_receiver.buffer[i]);
        }
        printf("\n");
#endif

        // 重置接收器
        uart_receiver_reset();
    }
}
void uart_data_handle(void)
{
    u8 recv_byte;
    u8 i;
    static u8 initialized = 0; // 串口接收器对象是否已经完成初始化

    // 初始化接收器（只执行一次）
    if (!initialized)
    {
        uart_receiver_reset();
        initialized = 1;
    }

    // 检查超时
    // if (uart_receiver.timeout_enable &&
    //     uart0_rxbuffer_get_count() == 0)
    if (uart_receiver.timeout_enable)
    {
        uart_receiver_timeout_handler();
    }

    if (uart0_rxbuffer_get_count() == 0)
    {
        return;
    }

    // 处理接收缓冲区中的数据
    while (1)
    {
        if (0 == uart0_rxbuffer_get_count() ||
            uart_receiver.status == UART_DATA_HANDLE_STATUS_FORMAT_TAIL)
        {
#if USER_DEBUG_ENABLE
            // 缓冲区为空或者接收完成，退出循环
            // if (0 == uart0_rxbuffer_get_count() && uart_receiver.status != UART_DATA_HANDLE_STATUS_FORMAT_TAIL)
            // {
            //     printf("0 == uart0_rxbuffer_get_count()\n");
            // }

            if (uart_receiver.status == UART_DATA_HANDLE_STATUS_FORMAT_TAIL)
            {
                printf("recv complete \n");
            }

#endif
            break;
        }

        recv_byte = uart0_rxbuffer_get_byte();

        // 启用超时计数
        uart_receiver.timeout_enable = 1;
        uart_receiver.timeout_cnt = 0;

        // 处理字节
        if (uart_receiver_process_byte(recv_byte))
        {
#if USER_DEBUG_ENABLE
            // 处理失败
            printf("Byte processing failed\n");
            printf("cur uart receiver status: %02d\n", (u16)uart_receiver.status);
            printf("cur uart receiver index: %02d\n", (u16)uart_receiver.index);
            printf("cur recved byte: 0x%02x\n", (u16)recv_byte);
#endif
            uart_receiver_reset();
        }
    }

    if (uart_receiver.status != UART_DATA_HANDLE_STATUS_FORMAT_TAIL)
    {
        return;
    }

    // ===========================================================
    // 接收完成，处理数据

#if USER_DEBUG_ENABLE
    // 打印接收到的一帧数据
    printf("================================>\n");
    printf("Received complete frame: \n");
    for (i = 0; i < uart_receiver.index; i++)
    {
        printf("0x%02x ", (u16)uart_receiver.buffer[i]);
    }
    printf("\n");
    printf("================================^\n");
#endif

    switch (uart_receiver.buffer[3])
    { // CMD字段在索引3位置
#if USER_DEBUG_ENABLE
    case 0x01:
        printf("Processing command 0x01\n");
        break;
    case 0x02:
        printf("Processing command 0x02\n");
        break;
    default:
        printf("Unknown command: 0x%02x\n", (u16)uart_receiver.buffer[3]);
        break;
#endif
    }

    // 重置接收器，准备下一次接收
    uart_receiver_reset();
}

// 根据命令，自动打包数据并发送
// USER_TO_DO 
void uart_data_send_cmd(uart_send_cmd_t cmd)
{
    switch (cmd)
    {
    // case constant expression:
    //     /* code */
    //     break;
    
    default:
        break;
    }
}
