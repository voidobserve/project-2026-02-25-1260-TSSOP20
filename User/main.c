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

void main(void)
{
    // 看门狗默认打开, 复位时间2s
    WDT_KEY = WDT_KEY_VAL(0xDD); // 关闭看门狗 (如需配置看门狗请查看"WDT\WDT_Reset"示例)

    system_init();

    // 关闭HCK和HDA的调试功能
    WDT_KEY = 0x55;  // 解除写保护
    IO_MAP &= ~0x01; // 清除这个寄存器的值，实现关闭HCK和HDA引脚的调试功能（解除映射）
    WDT_KEY = 0xBB;  // 写一个无效的数据，触发写保护

    timer0_init();
    timer1_init();
    adc_pin_init();
    adc_init();
    led_init();
    uart0_init();
    pwm_init();

    led_ctl_init();
    charge_det_init();

#if USER_DEBUG_ENABLE
    user_debug_pin_init();
    timebase_init();
    printf("sys reset\n");
    // printf("Battery Monitor Initialized\n");
    // printf("Voltage Range: %d-%d mV\n", BATTERY_VOLTAGE_MIN_MV, BATTERY_VOLTAGE_MAX_MV);
#endif

    while (1)
    {
        key_driver_scan(&ad_key_para);
        ad_key_handle();

        uart_data_handle();

        battery_monitor_handle();
        charge_det();


        // if (is_in_charging)
        // {
        //     printf("is in charging\n");
        // }
        // delay_ms(1000);



#if USER_DEBUG_ENABLE
        // user_test_main();
#endif
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2022 HUGE-IC ***** END OF FILE *****/