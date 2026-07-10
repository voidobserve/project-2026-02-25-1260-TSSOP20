#ifndef __LOW_POWER_H__
#define __LOW_POWER_H__ 

#include "user_include.h"

#define LOW_POWER_ENTER_TIME_WHEN_POWER_OFF ((u16)2000) // 关机后，多久进入低功耗，单位：ms

void low_power_handle(void);

void low_power_enter_timer_callback(void);

#endif