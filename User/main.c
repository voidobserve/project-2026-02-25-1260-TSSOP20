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
#include "my_config.h"
#include "uart0.h"
#include "timer0.h"
#include "adc.h"
#include "battery_monitor.h"  // 添加电池监测头文件

#include "uart_data_handle.h"
#include "ad_key.h"

// 电池检测相关变量
static u16 last_battery_adc_val = 0;
static u8 battery_check_counter = 0;
#define BATTERY_CHECK_INTERVAL 100  // 每100次循环检测一次电池

void debug_pin_init(void)
{
    P1_MD1 &= GPIO_P17_MODE_SEL(0x03); 
    P1_MD1 |= GPIO_P17_MODE_SEL(0x01); 
}

void battery_monitor_task(void)
{
    // 每隔一定时间检测电池状态
    battery_check_counter++;
    if (battery_check_counter >= BATTERY_CHECK_INTERVAL) {
        battery_check_counter = 0;
        
        // 检查电池ADC值是否更新
        if (adc_get_update_flag(ADC_CHANNEL_SEL_BAT_DET)) {
            u16 current_adc_val = adc_get_val(ADC_CHANNEL_SEL_BAT_DET);
            
            // 只有当ADC值发生变化时才处理
            if (current_adc_val != last_battery_adc_val) {
                last_battery_adc_val = current_adc_val;
                
                // 计算电池电压和电量
                u16 battery_voltage = ADC_TO_BATTERY_VOLTAGE_MV(current_adc_val);
                battery_level_t battery_level = get_battery_level_by_adc(current_adc_val);
                u8 battery_percentage = get_battery_percentage_by_adc(current_adc_val);
                
#if USE_MY_DEBUG
                printf("Battery: %d mV, %d%%, Level: %s\n", 
                       battery_voltage, 
                       battery_percentage,
                       get_battery_level_string(battery_level));
#endif
                
                // 可以在这里添加低电量警告逻辑
                if (battery_level == BATTERY_LEVEL_CRITICAL) {
                    // 添加低电量处理逻辑
                }
            }
            
            adc_clear_update_flag(ADC_CHANNEL_SEL_BAT_DET);
        }
    }
}

void main(void)
{
    // 看门狗默认打开, 复位时间2s
    WDT_KEY = WDT_KEY_VAL(0xDD); //  关闭看门狗 (如需配置看门狗请查看"WDT\WDT_Reset"示例)

    system_init();

    // 关闭HCK和HDA的调试功能
    WDT_KEY = 0x55;  // 解除写保护
    IO_MAP &= ~0x01; // 清除这个寄存器的值，实现关闭HCK和HDA引脚的调试功能（解除映射）
    WDT_KEY = 0xBB;  // 写一个无效的数据，触发写保护

    timer0_init();
    adc_pin_init();
    adc_init();
    uart0_init();
    

#if USE_MY_DEBUG
    debug_pin_init();
    printf("sys reset\n");
    printf("Battery Monitor Initialized\n");
    printf("Voltage Range: %d-%d mV\n", BATTERY_VOLTAGE_MIN_MV, BATTERY_VOLTAGE_MAX_MV);
#endif

    while (1)
    {   
        key_driver_scan(&ad_key_para);
        ad_key_handle();

        uart_data_handle();
        
        // // 添加电池监测任务
        // battery_monitor_task();
        
        // // ADC扫描任务
        // adc_scan();
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2022 HUGE-IC ***** END OF FILE *****/