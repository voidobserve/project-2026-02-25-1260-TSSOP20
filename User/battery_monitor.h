#ifndef __BATTERY_MONITOR_H__
#define __BATTERY_MONITOR_H__

#include "include.h"
#include "adc.h"
#include "user_include.h"

extern volatile u8 is_sent_low_bat_alert;

// 从低功耗唤醒 xx 时间后，再次初始化相关参数
extern volatile u8 is_low_power_wakeup_initialize_enable;
extern volatile u32 low_power_wakeup_initialize_cnt;

extern volatile u32 discharge_time_cnt; // 放电时间计数，单位：s
extern volatile u32 charge_time_cnt;	// 充电时间计数，单位：s

void battery_monitor_handle(void);

void batttery_monitor_1ms_isr(void);
void send_low_bat_timer_callback(void); // 控制发送低电量的周期

void bat_charge_time_cnt_update(u16 voltage_mv);
void bat_discharge_time_cnt_update(u16 voltage_mv);

#if USER_DEBUG_ENABLE
// void user_test_init_by_voltage_mv(u16 test_voltage_mv);
#endif

#endif