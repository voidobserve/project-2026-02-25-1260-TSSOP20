#ifndef __LED_H__
#define __LED_H__

#include "include.h"

#include "pwm.h"
#include "led_bat_lev.h" // REVIEW 

// 定义LED指示灯对应的驱动引脚
#define LED_PIN_100_PERCENT P13
#define LED_PIN_75_PERCENT P06
#define LED_PIN_50_PERCENT P05
#define LED_PIN_25_PERCENT P00

#define LED_100_PERCENT_ON() (LED_PIN_100_PERCENT = 1)
#define LED_100_PERCENT_OFF() (LED_PIN_100_PERCENT = 0)
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

#define LED_RED_PIN P02
#define LED_RED_ON() (LED_RED_PIN = 0)
#define LED_RED_OFF() (LED_RED_PIN = 1)

#define LED_BLUE_PIN P01
#define LED_BLUE_ON() (LED_BLUE_PIN = 0)
#define LED_BLUE_OFF() (LED_BLUE_PIN = 1)

// #define LED_YELLOW_SET_PWM_DUTY(channel_duty) pwm_set_channel_0_duty(channel_duty)
// #define LED_WHITE_SET_PWM_DUTY(channel_duty) pwm_set_channel_1_duty(channel_duty)

/*
    缓慢调整黄灯和白灯的亮度，由定时器调用
*/
#define PWM_DUTY_SLOW_ADJUST_TIME ((u32)300 * 1000) // 黄灯和白灯的pwm占空比缓慢调节时间，单位：ms

// 缓慢调节完成后，最终要调节到的占空比值，单位：百分比
// #define PWM_DEST_DUTY_PERCENT ((u8)100 - 60)
#define PWM_DEST_DUTY_PERCENT ((u8)100 - 65)
// 充电期间，最终要调节到的占空比值，单位：百分比
#define PWM_DEST_DUTY_PERCENT_DURING_CHARGING ((u8)100 - 35)

/**
 * @brief 每 xx ms调节1单位的占空比值。前提条件：占空比值小于调节时间
 *
 *      占空比值是 70 %，但是LED是引脚给低电平驱动，写成 100 - 70
 *      值 == 33
 *
 *      占空比值是 60 %，但是LED是引脚给低电平驱动，写成 100 - 60
 *      值 == 25
 */
#define PWM_DUTY_SLOW_ADJUST_UNIT     \
    ((u32)PWM_DUTY_SLOW_ADJUST_TIME / \
     PWM_DUTY_VAL_PERCENT_X(PWM_DEST_DUTY_PERCENT))

// TEST_ONLY
// enum
// {
//     val = PWM_DUTY_SLOW_ADJUST_UNIT,
// };

// 进入充电 或 退出充电，缓慢调节黄灯和白灯亮度的总时间。单位：ms
#define PWM_DUTY_SLOW_ADJUST_TIME_DURING_CHARGING ((u32)2 * 60 * 1000)
/**
 * @brief 充电期间，每 xx ms 调节1单位的占空比值
 *      目标占空比值是 35 %，但是LED是引脚给低电平驱动，写成 100 - 35
 *      目前值 == 6
 */
#define PWM_DUTY_SLOW_ADJUST_UNIT_DURING_CHARGING     \
    ((u32)PWM_DUTY_SLOW_ADJUST_TIME_DURING_CHARGING / \
     PWM_DUTY_VAL_PERCENT_X(PWM_DEST_DUTY_PERCENT_DURING_CHARGING))

// TEST_ONLY
// enum
// {
//     val = PWM_DUTY_SLOW_ADJUST_UNIT_DURING_CHARGING,
// };

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
typedef u8 led_status_t;

// #if 1
// // 电池电量指示灯状态：
// enum
// {
//     // LED_BAT_LEV_STA_ALERT = 0, // 低电量报警
//     LED_BAT_LEV_STA_1,         // 只亮 1颗 指示灯
//     LED_BAT_LEV_2,
//     LED_BAT_LEV_3,
//     LED_BAT_LEV_4,
// };
// typedef u8 led_bat_lev_sta_t;
// #endif

typedef struct
{
    led_status_t status; // 状态（红灯、蓝灯、黄灯和白灯）

    // 用于控制黄灯和白灯的pwm占空比：
    // u8 is_slowly_adjust_end; // 灯光缓慢调节是否结束
    u8 adjust_time_cnt;   // 灯光缓慢调节的时间计数（用来控制每隔 xx ms调节1单位的占空比值）
    u16 cur_pwm_duty_val; // 当前的PWM占空比数值

    u16 dest_pwm_duty_val; // 目标 PWM 占空比

    u32 working_time; // 灯光工作时间（根据这个时间来给黄灯、白灯缓慢降亮度）

    // 用于控制红灯和蓝灯闪烁的动画：
    u32 red_blue_flash_time_cnt; // 红灯、蓝灯闪烁的时间计数

} led_ctl_t;

// // 定义电池电量指示灯的各个状态
// enum
// {
//     LED_BAT_LEVEL_STA_IDLE,
//     LED_BAT_LEVEL_STA_CHARGE_BEGIN,      // 充电开始
//     LED_BAT_LEVEL_STA_CHARGE_BEGIN_ANIM, // 正在跑充电开始的动画
//     LED_BAT_LEVEL_STA_CHARGING,          // 充电中
//     LED_BAT_LEVEL_STA_CHARGE_END,        // 充电结束

//     LED_BAT_LEVEL_STA_DISCHARGE, // 放电中
// };
// typedef u8 led_bat_level_anim_sta_t;

extern volatile led_ctl_t led_ctl;
// extern volatile led_bat_level_anim_sta_t led_bat_level_sta; 

void led_init(void);

void led_yellow_on(void);
void led_yellow_off(void);
void led_white_on(void);
void led_white_off(void);

#if 0
void led_suspend(void);
void led_resume(void);
#endif

void led_ctl_init(void);
void led_status_switch(void);
void led_status_set(led_status_t status);

// void led_bat_lev_sta_init(u16 voltage_mv);

void led_red_blue_flash_1ms_isr(void);
void led_slow_adjust_isr(void);
// void led_bat_instruction_timer_callback(void);

#endif