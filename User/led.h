#ifndef __LED_H__
#define __LED_H__

#include "include.h"

#include "pwm.h"

// 定义LED指示灯对应的驱动引脚
#define LED_PIN_100_PERCENT P13
#define LED_PIN_75_PERCENT P06
#define LED_PIN_50_PERCENT P05
#define LED_PIN_25_PERCENT P00

#define LED_100_PERCENT_ON() (LED_PIN_100_PERCENT = 1)
#define LED_100_PERCENT_OFF() (LED_PIN_100_PERCNET = 0)
#define LED_100_PERCENT_TOGGLE() (LED_PIN_100_PERCENT ^= 1)

#define LED_75_PERCENT_ON() (LED_PIN_75_PERCENT = 1)
#define LED_75_PERCENT_OFF() (LED_PIN_75_PERCENT = 0)
#define LED_75_PERCENT_TOGGLE() (LED_PIN_75_PERCENT ^= 1)

#define LED_50_PERCENT_ON() (LED_PIN_50_PERCENT = 1)
#define LED_50_PERCENT_OFF() (LED_PIN_50_PERCENT = 0)
#define LED_50_PERCENT_TOGGLE() (LED_PIN_50_PERCENT ^= 1)

#define LED_25_PERCENT_ON() (LED_PIN_25_PERCENT = 1)
#define LED_25_PERCENT_OFF() (LED_PIN_25_PERCENT = 0)
#define LED_25_PERCENT_TOGGLE() (LED_PIN_25_PERCENT ^= 1)

// #define LED_YELLOW_SET_PWM_DUTY(channel_duty) pwm_set_channel_0_duty(channel_duty)
// #define LED_WHITE_SET_PWM_DUTY(channel_duty) pwm_set_channel_1_duty(channel_duty)

#define LED_YELLOW_ON() pwm_set_channel_0_duty(STRM0_PERIOD_30_PERCENT_VAL)
#define LED_YELLOW_OFF() pwm_set_channel_0_duty(STMR0_PERIOD_0_PERCENT_VAL)

#define LED_WHITE_ON() pwm_set_channel_1_duty(STRM1_PERIOD_30_PERCENT_VAL)
#define LED_WHITE_OFF() pwm_set_channel_1_duty(STMR1_PERIOD_0_PERCENT_VAL)

#define YELLOW_SLOW_START_TIME ((u32)30 * 1000)
// 缓启动期间，每次调节占空比的时间：（每次调节1单位的周期值，调节时间至少要大于等于1ms）
// #if (YELLOW_SLOW_START_TIME < STRM0_PERIOD_30_PERCENT_VAL)
// #define YELLOW_SLOW_START_ADJUST_TIME (STRM0_PERIOD_30_PERCENT_VAL / YELLOW_SLOW_START_TIME)
// #else
#define YELLOW_SLOW_START_ADJUST_TIME (YELLOW_SLOW_START_TIME / STRM0_PERIOD_30_PERCENT_VAL)
// #endif

/*
    灯光状态：
    暖灯亮->白光亮->白暖一起亮->红蓝闪->关
*/
enum
{
    LED_STATUS_OFF,
    LED_STATUS_YELLOW,
    LED_STATUS_WHITE,
    LED_STATUS_WHITE_YELLOW,
    LED_STATUS_RED_BLUE_FLASH,
};

typedef struct
{
    u8 status; // 状态

    u16 cur_period_val;
    u16 dest_period_val;

    u16 cnt; // 调节时间计数
} led_ctl_t;

void led_init(void);

void led_ctl_init(void);
void led_status_switch(void);

// void led_slow_start_isr(void);



#endif