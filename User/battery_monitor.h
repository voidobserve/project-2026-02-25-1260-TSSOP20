#ifndef __BATTERY_MONITOR_H__
#define __BATTERY_MONITOR_H__

#include "include.h"
#include "adc.h"

// 电池电压范围：2.5V - 4.2V
#define BATTERY_VOLTAGE_MIN_MV    2500    // 最低电压 2.5V
#define BATTERY_VOLTAGE_MAX_MV    4200    // 最高电压 4.2V
#define BATTERY_VOLTAGE_RANGE_MV  (BATTERY_VOLTAGE_MAX_MV - BATTERY_VOLTAGE_MIN_MV)  // 电压范围 1700mV

// 电池电量等级定义
typedef enum {
    BATTERY_LEVEL_EMPTY = 0,     // 0%   电压 < 2.5V (实际不会出现)
    BATTERY_LEVEL_CRITICAL,      // 1-25%  严重低电量
    BATTERY_LEVEL_LOW,           // 26-50% 低电量
    BATTERY_LEVEL_MEDIUM,        // 51-75% 中等电量
    BATTERY_LEVEL_HIGH,          // 76-100% 高电量
    BATTERY_LEVEL_FULL           // 100%   电压 = 4.2V
} battery_level_t;

// 电池电量百分比划分 (基于电压的线性映射)
// 由于锂电池放电曲线非线性，这里采用分段线性近似
#define BATTERY_VOLTAGE_100_PERCENT  4200  // 100% 对应 4.2V
#define BATTERY_VOLTAGE_75_PERCENT   3850  // 75%  对应 3.85V
#define BATTERY_VOLTAGE_50_PERCENT   3600  // 50%  对应 3.6V
#define BATTERY_VOLTAGE_25_PERCENT   3200  // 25%  对应 3.2V
#define BATTERY_VOLTAGE_0_PERCENT    2500  // 0%   对应 2.5V

// ADC相关参数 (电池检测使用内部2.0V参考电压，VDD 1/5分压)
#define BATTERY_ADC_REF_VOLTAGE_MV   2000  // 内部参考电压 2.0V
#define BATTERY_VOLTAGE_DIVIDER      5     // VDD经过1/5分压后输入ADC

// 计算ADC值对应的电池电压 (mV)
// ADC值 -> 实际电池电压 = ADC值 * (参考电压/4096) * 分压比 * 1000
#define ADC_TO_BATTERY_VOLTAGE_MV(adc_val) \
    (((u32)(adc_val) * BATTERY_ADC_REF_VOLTAGE_MV * BATTERY_VOLTAGE_DIVIDER) / 4096)

// 计算电池电压对应的电量等级
battery_level_t get_battery_level_by_voltage(u16 voltage_mv);

// 计算电池电压对应的百分比 (0-100)
u8 get_battery_percentage_by_voltage(u16 voltage_mv);

// 通过ADC值获取电池电量信息
battery_level_t get_battery_level_by_adc(u16 adc_val);
u8 get_battery_percentage_by_adc(u16 adc_val);

// 获取电池状态描述字符串
const char* get_battery_level_string(battery_level_t level);

#endif // __BATTERY_MONITOR_H__