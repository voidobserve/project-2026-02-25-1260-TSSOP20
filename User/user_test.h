#ifndef __USER_TEST_H__
#define __USER_TEST_H__

#include "include.h"
#include "user_config.h"
#if USER_DEBUG_ENABLE

// 时基ID枚举定义
enum
{
    TIMEBASE_ADC_SCAN = 0, // ADC扫描测试时基
    TIMEBASE_LED_TEST,     // LED测试时基
    TIMEBASE_UART_TEST,    // UART测试时基
    TIMEBASE_PWM_TEST,     // PWM测试时基
    TIMEBASE_MAX           // 最大时基数量
};
typedef u8 timebase_id_t;

// 时基配置结构体
typedef struct
{
    u16 interval_ms; // 时间间隔(毫秒)
    u16 counter;     // 计数器
    u8 enabled;      // 是否启用
    u8 flag;         // 触发标志
} timebase_config_t;

// 全局时基数组
extern timebase_config_t timebase_array[TIMEBASE_MAX];

extern volatile u8 flag_debug;

// 函数声明
void debug_time_add(void);
void timebase_init(void);
void timebaseUpdate(void);
void timebaseEnable(timebase_id_t id, u16 interval_ms);
void timebaseDisable(timebase_id_t id);
u8 isTimebaseTriggered(timebase_id_t id);
void clearTimebaseFlag(timebase_id_t id);

void user_test_adc_scan(void);
void user_test_led(void);

#endif

#endif