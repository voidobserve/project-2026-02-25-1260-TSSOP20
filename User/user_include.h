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
#include "low_power.h" // 低功耗

#if 0
// 需要延时再执行操作的任务ID
enum
{
    DELAY_EXEC_BLE_OFF,
    DELAY_EXEC_MAX,
};
typedef u8 delay_exec_task_id_t;

// 延时执行操作的控制块
typedef struct
{
    u8 dest_cnt;   // 目标延时计数
    u8 cur_cnt;    // 当前延时计数
    u8 is_pending; // 延时时间是否到来，挂起任务
    u8 is_enable;  // 当前任务是否启用
} delay_exec_block_t;

void delay_exec_set(delay_exec_task_id_t task_id, u8 interval_ms);
void delay_exec_reset(delay_exec_task_id_t task_id);
void delay_exec_update(void);
u8 delay_exec_is_pending(delay_exec_task_id_t task_id);
#endif

void user_init(void);

#endif