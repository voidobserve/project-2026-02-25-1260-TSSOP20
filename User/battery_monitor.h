#ifndef __BATTERY_MONITOR_H__
#define __BATTERY_MONITOR_H__

#include "include.h"
#include "adc.h"
#include "user_include.h"

// // 黄白灯一起亮（ YELLOW + WHITE ）
// #define BAT_WY_4LED_VOLTAGE ((u16)3700)
// #define BAT_WY_3LED_VOLTAGE ((u16)3600)
// #define BAT_WY_2LED_VOLTAGE ((u16)3500)
// #define BAT_WY_LOW_WARN_VOLTAGE ((u16)3250)
// // 死区电压
// #define BAT_WY_DEAD_ZONE_VOLTAGE ((u16)50)

// #if (!BAT_LED_TEST_ENABLE)
// // 放电时，电池电量指示灯只显示 3颗 时，对应的放电时间（从满电开始计算），单位：s
// #define BAT_WY_3LED_TIME ((u16)60 * 60)
// // 放电时，电池电量指示灯只显示 2颗 时，对应的放电时间（从满电开始计算），单位：s
// #define BAT_WY_2LED_TIME ((u16)120 * 60)
// // 放电时，电池电量指示灯只显示 1颗 时，对应的放电时间（从满电开始计算），单位：s
// #define BAT_WY_1LED_TIME ((u16)180 * 60)
// // 放电时，电池电量指示灯进行 低电量提示 时，对应的放电时间（从满电开始计算），单位：s
// #define BAT_WY_LOW_WARN_TIME ((u16)220 * 60)
// #else
// // 放电时，电池电量指示灯只显示 3颗 时，对应的放电时间（从满电开始计算），单位：s
// #define BAT_WY_3LED_TIME ((u16)6)
// // 放电时，电池电量指示灯只显示 2颗 时，对应的放电时间（从满电开始计算），单位：s
// #define BAT_WY_2LED_TIME ((u16)12)
// // 放电时，电池电量指示灯只显示 1颗 时，对应的放电时间（从满电开始计算），单位：s
// #define BAT_WY_1LED_TIME ((u16)18)
// // 放电时，电池电量指示灯进行 低电量提示 时，对应的放电时间（从满电开始计算），单位：s
// #define BAT_WY_LOW_WARN_TIME ((u16)22)
// #endif
// =========================================================================

// =========================================================================
// // 白灯或者黄灯单独亮（ WHITE or YELLOW only ）
// #define BAT_SINGLE_LIGHT_4LED_VOLTAGE ((u16)3800)
// #define BAT_SINGLE_LIGHT_3LED_VOLTAGE ((u16)3700)
// #define BAT_SINGLE_LIGHT_2LED_VOLTAGE ((u16)3600)
// #define BAT_SINGLE_LIGHT_LOW_WARN_VOLTAGE ((u16)3250)
// // 死区电压
// #define BAT_SINGLE_LIGHT_DEAD_ZONE_VOLTAGE ((u16)50)

// #if (!BAT_LED_TEST_ENABLE)
// // 放电时，电池电量指示灯只显示 3颗 时，对应的放电时间（从满电开始计算），单位：s
// #define BAT_SINGLE_LIGHT_3LED_TIME ((u16)90 * 60)
// #define BAT_SINGLE_LIGHT_2LED_TIME ((u16)170 * 60)
// #define BAT_SINGLE_LIGHT_1LED_TIME ((u16)260 * 60)
// #define BAT_SINGLE_LIGHT_LOW_WARN_TIME ((u16)310 * 60)
// #else
// // 放电时，电池电量指示灯只显示 3颗 时，对应的放电时间（从满电开始计算），单位：s
// #define BAT_SINGLE_LIGHT_3LED_TIME ((u16)9)
// #define BAT_SINGLE_LIGHT_2LED_TIME ((u16)17)
// #define BAT_SINGLE_LIGHT_1LED_TIME ((u16)26)
// #define BAT_SINGLE_LIGHT_LOW_WARN_TIME ((u16)31)
// #endif
// =========================================================================

// =========================================================================
// #if (!BAT_LED_TEST_ENABLE)
// // 充电时，电量各个指示灯所需的充电时间，单位：s
// // 备注：最后一个灯要等充电ic输出充满电的信号
// #define BAT_CHARGE_1LED_TIME ((u16)90 * 60)
// #define BAT_CHARGE_2LED_TIME ((u16)90 * 2 * 60)
// #define BAT_CHARGE_3LED_TIME ((u16)90 * 3 * 60)
// #define BAT_CHARGE_4LED_TIME ((u16)90 * 4 * 60) // 不使用这个时间，只作为计算使用
// #else
// // 充电时，电量各个指示灯所需的充电时间，单位：s
// // 备注：最后一个灯要等充电ic输出充满电的信号
// #define BAT_CHARGE_1LED_TIME ((u16)9)
// #define BAT_CHARGE_2LED_TIME ((u16)9 * 2)
// #define BAT_CHARGE_3LED_TIME ((u16)9 * 3)
// #define BAT_CHARGE_4LED_TIME ((u16)9 * 4) // 不使用这个时间，只作为计算使用
// #endif

extern volatile u8 is_sent_low_bat_alert;

extern volatile u8 is_in_low_bat_alert; // 是否处于低电量提示

extern volatile u32 discharge_time_cnt; // 放电时间计数，单位：s
extern volatile u32 charge_time_cnt;	// 充电时间计数，单位：s

void battery_monitor_handle(void);

void batttery_monitor_1ms_isr(void);
void send_low_bat_timer_callback(void); // 控制发送低电量的周期

// void bat_charge_time_cnt_update(u16 voltage_mv);
// void bat_discharge_time_cnt_update(u16 voltage_mv);

#if USER_DEBUG_ENABLE
// void user_test_init_by_voltage_mv(u16 test_voltage_mv);
#endif

#endif // __BATTERY_MONITOR_H__