#ifndef __BATTERY_MONITOR_H__
#define __BATTERY_MONITOR_H__

#include "include.h"
#include "adc.h"

// USER_TO_DO: 可能不使用下面这些宏
#define BATTERY_VOLTAGE_100_PERCENT 4180 // 100% 对应
#define BATTERY_VOLTAGE_75_PERCENT 4000  // 75%  对应
#define BATTERY_VOLTAGE_50_PERCENT 3850  // 50%  对应
#define BATTERY_VOLTAGE_25_PERCENT 3600  // 25%  对应
#define BATTERY_VOLTAGE_0_PERCENT 3300   // 0%   对应

// ADC相关参数 (电池检测使用内部2.0V参考电压，VDD 1/5分压)
#define BATTERY_ADC_REF_VOLTAGE_MV 2000 // 内部参考电压 2.0V
#define BATTERY_VOLTAGE_DIVIDER 5       // VDD经过1/5分压后输入ADC

// 计算ADC值对应的电池电压 (单位：mV)
// ADC值 -> 实际电池电压 = ADC值 * (参考电压/4096) * 分压比 * 1000
#define ADC_TO_BATTERY_VOLTAGE_MV(adc_val) \
    (((u32)(adc_val) * BATTERY_ADC_REF_VOLTAGE_MV * BATTERY_VOLTAGE_DIVIDER) / 4096)

// 检测电池电压的周期，单位：ms
#define BATTERY_MONITOR_TIME_PERIOD ((u16)2000)

extern volatile u8 is_battery_monitor_time_comes; // 控制函数调用周期的变量

extern volatile u8 stable_bat_percent;

typedef struct
{
    u8 battery_percent;

} battery_monitor_t;

void send_low_battery_timer_callback(void);
void refresh_battery_level_timer_callback(void);

// 计算电池电压对应的百分比 (0-100)
u8 get_battery_percentage_by_voltage(u16 voltage_mv);

// 通过ADC值获取电池电量信息
// battery_level_t get_battery_level_by_adc(u16 adc_val);
u8 get_battery_percentage_by_adc(u16 adc_val);

void battery_monitor_handle(void);

#endif // __BATTERY_MONITOR_H__