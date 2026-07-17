#ifndef __LOW_POWER_H__
#define __LOW_POWER_H__

#include "user_include.h"

// 关机后，多久进入低功耗，单位：ms
// #define LOW_POWER_ENTER_TIME_WHEN_POWER_OFF ((u32)2 * 60 * 1000)
#define LOW_POWER_ENTER_TIME_WHEN_POWER_OFF ((u32)2 * 1000)

void low_power_handle(void);

void low_power_enter_timer_callback(void);

#endif