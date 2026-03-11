#ifndef __USER_INCLUDE_H__
#define __USER_INCLUDE_H__

#include "user_config.h"
#include "led.h"
#include "uart0.h"
#include "adc.h"
#include "pwm.h"

#include "timer0.h"
#include "timer1.h"

#if USER_DEBUG_ENABLE
#include "user_test.h"
#endif

#include "key_driver.h"
#include "ad_key.h"
#include "battery_monitor.h"
#include "uart_data_handle.h"
#include "bluetooth_ic_handle.h"  
#include "charge_det.h"   

#endif