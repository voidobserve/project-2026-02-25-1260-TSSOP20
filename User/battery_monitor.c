#include "battery_monitor.h"
#include "user_include.h"

// USER_TO_DO 关机之后需要清空这个标志位：
volatile u8 is_send_low_battery = 0; // 是否发送过低电压的提示
volatile u8 is_low_battery = 0;      // 是否处于低电量状态

// 根据电压值获取电池电量等级
// battery_level_t get_battery_level_by_voltage(u16 voltage_mv)
// {
//     if (voltage_mv >= BATTERY_VOLTAGE_100_PERCENT)
//     {
//         return BATTERY_LEVEL_FULL;
//     }
//     else if (voltage_mv >= BATTERY_VOLTAGE_75_PERCENT)
//     {
//         return BATTERY_LEVEL_HIGH;
//     }
//     else if (voltage_mv >= BATTERY_VOLTAGE_50_PERCENT)
//     {
//         return BATTERY_LEVEL_MEDIUM;
//     }
//     else if (voltage_mv >= BATTERY_VOLTAGE_25_PERCENT)
//     {
//         return BATTERY_LEVEL_LOW;
//     }
//     else if (voltage_mv >= BATTERY_VOLTAGE_0_PERCENT)
//     {
//         return BATTERY_LEVEL_CRITICAL;
//     }
//     else
//     {
//         return BATTERY_LEVEL_EMPTY;
//     }
// }

// 根据电压值计算电池电量百分比 (0-100)
u8 get_battery_percentage_by_voltage(u16 voltage_mv)
{
    if (voltage_mv >= BATTERY_VOLTAGE_100_PERCENT)
    {
        return 100;
    }
    else if (voltage_mv <= BATTERY_VOLTAGE_0_PERCENT)
    {
        return 0;
    }

    // 线性插值计算百分比
    if (voltage_mv >= BATTERY_VOLTAGE_75_PERCENT)
    {
        // 100% - 75% 区间: 4.2V - 3.85V
        return 75 + ((u32)(voltage_mv - BATTERY_VOLTAGE_75_PERCENT) * 25) /
                        (BATTERY_VOLTAGE_100_PERCENT - BATTERY_VOLTAGE_75_PERCENT);
    }
    else if (voltage_mv >= BATTERY_VOLTAGE_50_PERCENT)
    {
        // 75% - 50% 区间: 3.85V - 3.6V
        return 50 + ((u32)(voltage_mv - BATTERY_VOLTAGE_50_PERCENT) * 25) /
                        (BATTERY_VOLTAGE_75_PERCENT - BATTERY_VOLTAGE_50_PERCENT);
    }
    else if (voltage_mv >= BATTERY_VOLTAGE_25_PERCENT)
    {
        // 50% - 25% 区间: 3.6V - 3.2V
        return 25 + ((u32)(voltage_mv - BATTERY_VOLTAGE_25_PERCENT) * 25) /
                        (BATTERY_VOLTAGE_50_PERCENT - BATTERY_VOLTAGE_25_PERCENT);
    }
    else
    {
        // 25% - 0% 区间: 3.2V - 2.5V
        return ((u32)(voltage_mv - BATTERY_VOLTAGE_0_PERCENT) * 25) /
               (BATTERY_VOLTAGE_25_PERCENT - BATTERY_VOLTAGE_0_PERCENT);
    }
}

// // 根据ADC值获取电池电量等级
// battery_level_t get_battery_level_by_adc(u16 adc_val)
// {
//     u16 voltage_mv = ADC_TO_BATTERY_VOLTAGE_MV(adc_val);
//     return get_battery_level_by_voltage(voltage_mv);
// }

// 根据ADC值计算电池电量百分比
u8 get_battery_percentage_by_adc(u16 adc_val)
{
    u16 voltage_mv = ADC_TO_BATTERY_VOLTAGE_MV(adc_val);
    // printf("voltage_mv == %u\n", voltage_mv); // 打印转换好的电压值
    return get_battery_percentage_by_voltage(voltage_mv);
}

// 电池电压划分建议说明
/*
电池电压范围：2.5V - 4.2V 的划分方案：

1. 电压区间划分（考虑锂电池特性）：
   - 100% (满电):    4.20V
   - 75% (高电量):   3.85V
   - 50% (中电量):   3.60V
   - 25% (低电量):   3.20V
   - 0%  (耗尽):     2.50V

2. 划分依据：
   - 锂电池放电曲线特性：电压下降不是完全线性的
   - 实际使用经验：大部分使用时间集中在3.2V-4.2V之间
   - 安全考虑：避免过度放电损坏电池

3. ADC值对应关系（使用内部2.0V参考电压，1/5分压）：
   - 4.20V → ADC值约 2048
   - 3.85V → ADC值约 1876
   - 3.60V → ADC值约 1755
   - 3.20V → ADC值约 1560
   - 2.50V → ADC值约 1221

4. 使用建议：
   - 可以根据实际电池型号调整这些电压值
   - 建议添加滞回比较防止电量显示抖动
   - 低电量时应该给出警告提示
*/

void battery_monitor_init(void)
{
}

void battery_monitor_handle(void)
{
    static u16 adc_val = 0;
    u8 bat_percent = 0; // 电池电量百分比

    if (adc_get_update_flag(ADC_CHANNEL_SEL_BAT_DET))
    {
        adc_clear_update_flag(ADC_CHANNEL_SEL_BAT_DET);
        adc_val = adc_get_val(ADC_CHANNEL_SEL_BAT_DET);

        // printf("bat adc val == %u\n", adc_val);
    }

    // USER_TO_DO
    /*
        充电时、放电时、太阳能一侧的电压比电池电压还大时，才进行电池电量显示
    */
    if ((led_ctl.status == LED_STATUS_OFF) &&
        (ble_ic.is_working == 0) &&
        (is_in_charging_by_charger == 0) &&
        (is_in_charging_by_solar_panel == 0))
    {
        // 不需要电量显示的场合，把电量指示灯全部关闭
        LED_25_PERCENT_OFF();
        LED_50_PERCENT_OFF();
        LED_75_PERCENT_OFF();
        LED_100_PERCENT_OFF();
        return;
    }

    // USER_TO_DO 这里需要加入滞回比较防止电量显示抖动
    // 根据ad值直接获取电池电量百分比
    bat_percent = get_battery_percentage_by_adc(adc_val);
    // printf("bat_percnent == %u\n", (u16)bat_percent);

    if (bat_percent <= 0)
    {
        // 低电量关机

        // 给蓝牙ic发送低电量关机
        uart_data_send_cmd(UART_SEND_CMD_LOW_POWER_SHUTDOWN);

        // 关闭电量指示灯
        LED_25_PERCENT_OFF();
        LED_50_PERCENT_OFF();
        LED_75_PERCENT_OFF();
        LED_100_PERCENT_OFF();

        // 关闭驱动的灯光
        led_ctl.status = LED_STATUS_OFF;
        LED_WHITE_OFF();
        LED_YELLOW_OFF();
        P02 = 1;
        P01 = 1;

        is_send_low_battery = 0;
        is_low_battery = 1;
        // 之后在串口等待蓝牙ic回复已经关闭功放的数据，再关闭蓝牙
        return;
    }
    else if (bat_percent <= 25 && is_send_low_battery == 0)
    {
        // 低电量，并且没有发送低电量的提示
        uart_data_send_cmd(UART_SEND_CMD_LOW_POWER_WARNING);
        is_send_low_battery = 1;
        is_low_battery = 1;
    }
    else if (bat_percent >= 30)
    {
        is_send_low_battery = 0;
    }

    if (bat_percent >= 25)
    {
        LED_25_PERCENT_ON();
    }

    if (bat_percent >= 50)
    {
        LED_50_PERCENT_ON();
    }
    else
    {
        LED_50_PERCENT_OFF();
    }

    if (bat_percent >= 75)
    {
        LED_75_PERCENT_ON();
    }
    else
    {
        LED_75_PERCENT_OFF();
    }

    if (bat_percent >= 100)
    {
        LED_100_PERCENT_ON();
    }
    else
    {
        LED_100_PERCENT_OFF();
    }
}
