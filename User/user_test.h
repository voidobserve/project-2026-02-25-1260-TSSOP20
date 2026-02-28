#ifndef __USER_TEST_H__
#define __USER_TEST_H__

#include "include.h"
#include "user_config.h"
#if USER_DEBUG_ENABLE

#define USER_DEBUG_PIN P17
#define USER_DEBUG_PIN_SET() (USER_DEBUG_PIN = 1)
#define USER_DEBUG_PIN_RESET() (USER_DEBUG_PIN = 0)
#define USER_DEBUG_PIN_TOGGLE() (USER_DEBUG_PIN ^= 1)


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

void user_debug_pin_init(void);

// 函数声明
void debug_time_add(void);

void timebase_init(void);
void timebase_update(void);
void timebase_enable(timebase_id_t id, u16 interval_ms);
void timebase_disable(timebase_id_t id);
u8 is_timebase_triggered(timebase_id_t id);
void timebase_clear_flag(timebase_id_t id);

void user_test_adc_scan(void);
void user_test_led(void);

void user_test_main(void);

#endif

#endif