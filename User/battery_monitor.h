#ifndef __BATTERY_MONITOR_H__
#define __BATTERY_MONITOR_H__

#include "include.h"
#include "adc.h"
#include "user_include.h"

extern volatile u8 is_sent_low_bat_alert;

void battery_monitor_handle(void);

void send_low_bat_timer_callback(void); // 控制发送低电量的周期

#if USER_DEBUG_ENABLE
// void user_test_init_by_voltage_mv(u16 test_voltage_mv);
#endif

#endif // __BATTERY_MONITOR_H__