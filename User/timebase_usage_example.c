/*
 * 时基管理系统使用示例
 * 这个文件展示了如何在主程序中使用时基管理系统
 */

#include "user_test.h"
#include "timer0.h"  // 假设定时器0用于提供基础时基

#if USER_DEBUG_ENABLE

/**
 * @brief 主程序初始化时基系统
 */
void main_program_init(void)
{
    // 初始化时基管理系统
    timebase_init();
    
    // 可以根据需要动态调整时基配置
    timebaseEnable(TIMEBASE_UART_TEST, 200);   // UART测试每200ms执行一次
    timebaseDisable(TIMEBASE_PWM_TEST);        // 暂时禁用PWM测试
}

/**
 * @brief 在定时器中断或主循环中调用时基更新
 * 建议在1ms定时器中断中调用
 */
void main_loop_timebase_handler(void)
{
    // 每1ms调用一次
    timebaseUpdate();
}

/**
 * @brief 主循环中的测试函数调用示例
 */
void main_loop_test_functions(void)
{
    // ADC扫描测试
    user_test_adc_scan();
    
    // LED测试
    user_test_led();
    
    // 可以添加更多测试函数
    // user_test_uart();
    // user_test_pwm();
}

/**
 * @brief 动态时基控制示例
 */
void dynamic_timebase_control(void)
{
    static u8 test_mode = 0;
    
    switch(test_mode)
    {
        case 0:
            // 正常测试模式
            timebaseEnable(TIMEBASE_ADC_SCAN, 1000);
            timebaseEnable(TIMEBASE_LED_TEST, 500);
            break;
            
        case 1:
            // 高频测试模式
            timebaseEnable(TIMEBASE_ADC_SCAN, 100);
            timebaseEnable(TIMEBASE_LED_TEST, 100);
            break;
            
        case 2:
            // 节能测试模式
            timebaseEnable(TIMEBASE_ADC_SCAN, 5000);
            timebaseDisable(TIMEBASE_LED_TEST);
            break;
    }
}

/**
 * @brief 获取时基状态信息(用于调试)
 */
void print_timebase_status(void)
{
    u8 i;
    printf("=== Timebase Status ===\r\n");
    for (i = 0; i < TIMEBASE_MAX; i++)
    {
        printf("Timebase %d: Enabled=%d, Interval=%dms, Counter=%d, Flag=%d\r\n",
               i,
               timebase_array[i].enabled,
               timebase_array[i].interval_ms,
               timebase_array[i].counter,
               timebase_array[i].flag);
    }
}

#endif