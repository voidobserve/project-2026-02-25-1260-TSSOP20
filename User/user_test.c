#include "user_test.h"
#include <stdio.h>

#include "adc.h"
#include "led.h"

#if USER_DEBUG_ENABLE

volatile u8 flag_debug = 0;
timebase_config_t timebase_array[TIMEBASE_MAX];

void debug_time_add(void)
{
#if 1
    static u16 cnt = 0;
    cnt++;
    if (cnt >= 1000)
    {
        cnt = 0;
        flag_debug = 1;
    }
#endif
}

/**
 * @brief 初始化时基管理系统
 */
void timebase_init(void)
{
    u8 i;
    for (i = 0; i < TIMEBASE_MAX; i++)
    {
        timebase_array[i].interval_ms = 0;
        timebase_array[i].counter = 0;
        timebase_array[i].enabled = 0;
        timebase_array[i].flag = 0;
    }

    // 默认配置示例
    timebaseEnable(TIMEBASE_ADC_SCAN, 1000); // ADC扫描每1秒执行一次
    timebaseEnable(TIMEBASE_LED_TEST, 500);  // LED测试每500ms执行一次
}

/**
 * @brief 更新时基计数器，在主循环或定时器中断中调用
 */
void timebaseUpdate(void)
{
    u8 i;
    for (i = 0; i < TIMEBASE_MAX; i++)
    {
        if (timebase_array[i].enabled)
        {
            timebase_array[i].counter++;
            if (timebase_array[i].counter >= timebase_array[i].interval_ms)
            {
                timebase_array[i].counter = 0;
                timebase_array[i].flag = 1;
            }
        }
    }
}

/**
 * @brief 启用指定的时基
 * @param id 时基ID
 * @param interval_ms 执行间隔(毫秒)
 */
void timebaseEnable(timebase_id_t id, u16 interval_ms)
{
    if (id < TIMEBASE_MAX)
    {
        timebase_array[id].interval_ms = interval_ms;
        timebase_array[id].counter = 0;
        timebase_array[id].enabled = 1;
        timebase_array[id].flag = 0;
    }
}

/**
 * @brief 禁用指定的时基
 * @param id 时基ID
 */
void timebaseDisable(timebase_id_t id)
{
    if (id < TIMEBASE_MAX)
    {
        timebase_array[id].enabled = 0;
        timebase_array[id].flag = 0;
    }
}

/**
 * @brief 检查时基是否触发
 * @param id 时基ID
 * @return 1-已触发, 0-未触发
 */
u8 isTimebaseTriggered(timebase_id_t id)
{
    if (id < TIMEBASE_MAX)
    {
        return timebase_array[id].flag;
    }
    return 0;
}

/**
 * @brief 清除时基触发标志
 * @param id 时基ID
 */
void clearTimebaseFlag(timebase_id_t id)
{
    if (id < TIMEBASE_MAX)
    {
        timebase_array[id].flag = 0;
    }
}

// if (adc_get_update_flag(ADC_CHANNEL_SEL_AD_KEY))
// {
//     adc_clear_update_flag(ADC_CHANNEL_SEL_AD_KEY);
//     adkey_val = adc_get_val(ADC_CHANNEL_SEL_AD_KEY);
//     printf("adkey_val == %u\n", adkey_val);
//     delay_ms(500);
// }

// printf("adkey 1 press val == %u\n", AD_KEY_1_PRESS_VAL);
// printf("adkey 2 press val == %u\n", AD_KEY_2_PRESS_VAL);
// printf("adkey 3 press val == %u\n", AD_KEY_3_PRESS_VAL);
// printf("adkey 4 press val == %u\n", AD_KEY_4_PRESS_VAL);
// printf("adkey index 1 val == %u\n", AD_KEY_INDEX_1_VAL);
// printf("adkey index 2 val == %u\n", AD_KEY_INDEX_2_VAL);
// printf("adkey index 3 val == %u\n", AD_KEY_INDEX_3_VAL);
// printf("adkey index 4 val == %u\n", AD_KEY_INDEX_4_VAL);
// delay_ms(500);

// 测试adc的采集功能
void user_test_adc_scan(void)
{
    static u8 dir = 0;
    u16 adc_val = 0;

    // 使用新的时基系统
    if (isTimebaseTriggered(TIMEBASE_ADC_SCAN))
    {
        clearTimebaseFlag(TIMEBASE_ADC_SCAN);

        if (dir == 0)
        {
            if (adc_get_update_flag(ADC_CHANNEL_SEL_AD_KEY))
            {
                adc_clear_update_flag(ADC_CHANNEL_SEL_AD_KEY);
                adc_val = adc_get_val(ADC_CHANNEL_SEL_AD_KEY);
                printf("adkey_val == %u\n", adc_val);
                dir = 1;
            }
        }
        else if (dir == 1)
        {
            if (adc_get_update_flag(ADC_CHANNEL_SEL_SOLAR_DET))
            {
                adc_clear_update_flag(ADC_CHANNEL_SEL_SOLAR_DET);
                adc_val = adc_get_val(ADC_CHANNEL_SEL_SOLAR_DET);
                printf("solar val == %u\n", adc_val);
                dir = 2;
            }
        }
        else if (dir == 2)
        {
            if (adc_get_update_flag(ADC_CHANNEL_SEL_BAT_DET))
            {
                adc_clear_update_flag(ADC_CHANNEL_SEL_BAT_DET);
                adc_val = adc_get_val(ADC_CHANNEL_SEL_BAT_DET);
                printf("bat val == %u\n", adc_val);
                dir = 0;
            }
        }
    }
}

void user_test_led(void)
{
    // 使用LED测试时基
    if (isTimebaseTriggered(TIMEBASE_LED_TEST))
    {
        clearTimebaseFlag(TIMEBASE_LED_TEST);
        LED_100_PERCENT_TOGGLE();
        LED_75_PERCENT_TOGGLE();
        LED_50_PERCENT_TOGGLE();
        LED_25_PERCENT_TOGGLE();
    }
}

#endif