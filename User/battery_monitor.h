#ifndef __BATTERY_MONITOR_H__
#define __BATTERY_MONITOR_H__

#include "include.h"
#include "adc.h"
#include "user_include.h"

// 电池模型参数
#define BATTERY_FULL_VOLTAGE 4200        // 满电电压 (mV)
#define BATTERY_EMPTY_VOLTAGE 3300       // 空电电压 (mV)
#define BATTERY_LOW_WARNING_VOLTAGE 3600 // 低电量警告电压 (mV)

// ADC相关参数 (电池检测使用内部2.0V参考电压，VDD 1/5分压)
#define BATTERY_ADC_REF_VOLTAGE_MV 2000 // 内部参考电压 2.0V
#define BATTERY_VOLTAGE_DIVIDER 5       // VDD经过1/5分压后输入ADC

// 计算ADC值对应的电池电压 (单位：mV)
// ADC值 -> 实际电池电压 = ADC值 * (参考电压/4096) * 分压比 * 1000
#define ADC_TO_BATTERY_VOLTAGE_MV(adc_val) \
    (((u32)(adc_val) * BATTERY_ADC_REF_VOLTAGE_MV * BATTERY_VOLTAGE_DIVIDER) / 4096)

#define VOLTAGE_HISTORY_SIZE 10
// 检测电池电压的周期，单位：ms
#define BATTERY_VOLTAGE_UPDATE_PERIOD ((u16)4000)
// 每隔多久将采集的电池电压放入缓存中，单位：ms
#define BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER ((u32)15 * 1000)
// 每隔多久将缓存中的电池电压提取出来，单位：ms
#define BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER_EXTRACT \
    (BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER * VOLTAGE_HISTORY_SIZE)

// 电池电量更新模块的状态
enum
{
    BAT_VOL_UPDATE_STA_IDLE,      // 空闲
    BAT_VOL_UPDATE_STA_CAPTURING, // 正在采集
    BAT_VOL_UPDATE_STA_COMPLETED, // 采集完成
}; // battery voltage update status
typedef u8 bat_vol_update_sta_t;

// extern volatile u8 is_send_low_battery_enable;

extern volatile u8 bat_percent;
extern volatile u8 is_sent_low_bat_alert;
extern volatile u8 is_turn_off_by_low_bat; // 是否低电量关机

// void send_low_battery_timer_callback(void);

// 计算电池电压对应的百分比 (0-100)
u8 get_battery_percentage_by_voltage(u16 voltage_mv);
// void battery_monitor_init_by_adc_val(u16 adc_val);
void battery_monitor_refresh_by_adc_val(u16 adc_val);
void battery_monitor_handle(void);

void bat_vol_update_timer_callback(void);
void bat_vol_buff_add_timer_callback(void);
void bat_vol_buff_get_avg_timer_callback(void);

void send_low_bat_timer_callback(void); // 控制发送低电量的周期

#if USER_DEBUG_ENABLE
void user_test_init_by_voltage_mv(u16 test_voltage_mv);
#endif

#endif // __BATTERY_MONITOR_H__