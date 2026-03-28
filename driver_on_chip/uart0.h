#ifndef __UART0_H__
#define __UART0_H__
 
#include "include.h" // 芯片官方提供的头文件


#define UART0_BAUD 115200
#define USER_UART0_BAUD ((SYSCLK - UART0_BAUD) / (UART0_BAUD))

#define UART_RX_BUF_SIZE 128

// 增加环形缓冲区结构体
typedef struct
{
	u8 buffer[UART_RX_BUF_SIZE];
	u8 head;
	u8 tail;
	u8 count;
} uart_rx_buffer_t;

void uart0_init(void);

void uart0_sendbyte(u8 byte);
void uart0_sendbuf(u8 *buf, u8 len);

u8 uart0_rxbuffer_get_count(void);
u8 uart0_rxbuffer_get_byte(void);


#endif
