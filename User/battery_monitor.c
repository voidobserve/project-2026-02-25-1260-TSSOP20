#include "battery_monitor.h"
#include "my_config.h"

// 根据电压值获取电池电量等级
battery_level_t get_battery_level_by_voltage(u16 voltage_mv)
{
    if (voltage_mv >= BATTERY_VOLTAGE_100_PERCENT) {
        return BATTERY_LEVEL_FULL;
    } else if (voltage_mv >= BATTERY_VOLTAGE_75_PERCENT) {
        return BATTERY_LEVEL_HIGH;
    } else if (voltage_mv >= BATTERY_VOLTAGE_50_PERCENT) {
        return BATTERY_LEVEL_MEDIUM;
    } else if (voltage_mv >= BATTERY_VOLTAGE_25_PERCENT) {
        return BATTERY_LEVEL_LOW;
    } else if (voltage_mv >= BATTERY_VOLTAGE_0_PERCENT) {
        return BATTERY_LEVEL_CRITICAL;
    } else {
        return BATTERY_LEVEL_EMPTY;
    }
}

// 根据电压值计算电池电量百分比 (0-100)
u8 get_battery_percentage_by_voltage(u16 voltage_mv)
{
    if (voltage_mv >= BATTERY_VOLTAGE_100_PERCENT) {
        return 100;
    } else if (voltage_mv <= BATTERY_VOLTAGE_0_PERCENT) {
        return 0;
    }
    
    // 线性插值计算百分比
    if (voltage_mv >= BATTERY_VOLTAGE_75_PERCENT) {
        // 100% - 75% 区间: 4.2V - 3.85V
        return 75 + ((u32)(voltage_mv - BATTERY_VOLTAGE_75_PERCENT) * 25) / 
                    (BATTERY_VOLTAGE_100_PERCENT - BATTERY_VOLTAGE_75_PERCENT);
    } else if (voltage_mv >= BATTERY_VOLTAGE_50_PERCENT) {
        // 75% - 50% 区间: 3.85V - 3.6V
        return 50 + ((u32)(voltage_mv - BATTERY_VOLTAGE_50_PERCENT) * 25) / 
                    (BATTERY_VOLTAGE_75_PERCENT - BATTERY_VOLTAGE_50_PERCENT);
    } else if (voltage_mv >= BATTERY_VOLTAGE_25_PERCENT) {
        // 50% - 25% 区间: 3.6V - 3.2V
        return 25 + ((u32)(voltage_mv - BATTERY_VOLTAGE_25_PERCENT) * 25) / 
                    (BATTERY_VOLTAGE_50_PERCENT - BATTERY_VOLTAGE_25_PERCENT);
    } else {
        // 25% - 0% 区间: 3.2V - 2.5V
        return ((u32)(voltage_mv - BATTERY_VOLTAGE_0_PERCENT) * 25) / 
               (BATTERY_VOLTAGE_25_PERCENT - BATTERY_VOLTAGE_0_PERCENT);
    }
}

// 根据ADC值获取电池电量等级
battery_level_t get_battery_level_by_adc(u16 adc_val)
{
    u16 voltage_mv = ADC_TO_BATTERY_VOLTAGE_MV(adc_val);
    return get_battery_level_by_voltage(voltage_mv);
}

// 根据ADC值计算电池电量百分比
u8 get_battery_percentage_by_adc(u16 adc_val)
{
    u16 voltage_mv = ADC_TO_BATTERY_VOLTAGE_MV(adc_val);
    return get_battery_percentage_by_voltage(voltage_mv);
}

// 获取电池状态描述字符串
const char* get_battery_level_string(battery_level_t level)
{
    switch (level) {
        case BATTERY_LEVEL_EMPTY:
            return "EMPTY";
        case BATTERY_LEVEL_CRITICAL:
            return "CRITICAL";
        case BATTERY_LEVEL_LOW:
            return "LOW";
        case BATTERY_LEVEL_MEDIUM:
            return "MEDIUM";
        case BATTERY_LEVEL_HIGH:
            return "HIGH";
        case BATTERY_LEVEL_FULL:
            return "FULL";
        default:
            return "UNKNOWN";
    }
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