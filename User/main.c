/**
 ******************************************************************************
 * @file    main.c
 * @author  HUGE-IC Application Team
 * @version V1.0.0
 * @date    02-09-2022
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2021 HUGE-IC</center></h2>
 *
 * 版权说明后续补上
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "include.h"
#include "user_config.h"
#include "uart0.h"
#include "timer0.h"
#include "adc.h"
#include "led.h"
#include "pwm.h"

// #include "battery_monitor.h"  // 添加电池监测头文件

#include "uart_data_handle.h"
#include "ad_key.h"

#include "user_include.h"

#if 0
volatile delay_exec_block_t delay_exec_array[DELAY_EXEC_MAX];

void delay_exec_init(void)
{
    u8 i;
    for (i = 0; i < DELAY_EXEC_MAX; i++)
    {
        delay_exec_array[i].cur_cnt = 0;
        delay_exec_array[i].dest_cnt = 0;
        delay_exec_array[i].is_pending = 0;
        delay_exec_array[i].is_enable = 0;
    }
}

void delay_exec_set(delay_exec_task_id_t task_id, u8 interval_ms)
{
    delay_exec_array[task_id].cur_cnt = 0;
    delay_exec_array[task_id].dest_cnt = interval_ms;
    delay_exec_array[task_id].is_pending = 0;
    delay_exec_array[task_id].is_enable = 1;
}

void delay_exec_reset(delay_exec_task_id_t task_id)
{
    delay_exec_array[task_id].is_enable = 0;
    delay_exec_array[task_id].cur_cnt = 0;
    delay_exec_array[task_id].dest_cnt = 0;
    delay_exec_array[task_id].is_pending = 0;
}

// 更新延时执行任务的当前时间计数
void delay_exec_update(void)
{
    u8 i;
    for (i = 0; i < DELAY_EXEC_MAX; i++)
    {
        if (delay_exec_array[i].is_enable)
        {
            delay_exec_array[i].cur_cnt++;
            if (delay_exec_array[i].cur_cnt >= delay_exec_array[i].dest_cnt)
            {
                delay_exec_array[i].cur_cnt = 0;
                delay_exec_array[i].is_pending = 1;
            }
        }
    }
}

u8 delay_exec_is_pending(delay_exec_task_id_t task_id)
{
    return delay_exec_array[task_id].is_pending;
}

void delay_exec_handle(void)
{
    if (delay_exec_is_pending(DELAY_EXEC_BLE_OFF))
    {
        // delay_exec_reset(DELAY_EXEC_BLE_OFF);

        delay_exec_array[DELAY_EXEC_BLE_OFF].is_pending = 0;

        // 关闭蓝牙
        USER_DEBUG_PIN_TOGGLE();
    }
}
#endif

void user_init(void)
{
    adc_pin_init();
    adc_init();
    led_init();
    uart0_init();
    pwm_init();
    bluetooth_ic_handle_init();

    led_ctl_init();
    charge_det_init();
    // delay_exec_init();

    // 需要等外设都准备好，再跑时间
    timer0_init();
    timer1_init();

#if USER_DEBUG_ENABLE
    user_debug_pin_init();
    // timebase_init();
    printf("sys init\n");
#endif
}

void main(void)
{
    // 看门狗默认打开, 复位时间2s
    WDT_KEY = WDT_KEY_VAL(0xDD); // 关闭看门狗 (如需配置看门狗请查看"WDT\WDT_Reset"示例)
    system_init();

    // 关闭HCK和HDA的调试功能
    WDT_KEY = 0x55;  // 解除写保护
    IO_MAP &= ~0x01; // 清除这个寄存器的值，实现关闭HCK和HDA引脚的调试功能（解除映射）
    WDT_KEY = 0xBB;  // 写一个无效的数据，触发写保护

    user_init();
    delay_ms(10); // 等待系统稳定（至少要等ad值都更新一遍）

    while (1)
    {
        key_driver_scan(&ad_key_para);
        ad_key_handle();

        uart_data_handle();

        battery_monitor_handle();

        charge_det();

        // low_power_handle(); 

#if USER_DEBUG_ENABLE
        // user_test_main();
#endif
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2022 HUGE-IC ***** END OF FILE *****/