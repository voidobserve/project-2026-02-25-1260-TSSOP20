#ifndef __BATTERY_MONITOR_H__
#define __BATTERY_MONITOR_H__

#include "include.h"
#include "adc.h"
#include "user_include.h"

// 电池模型参数
#define BATTERY_FULL_VOLTAGE 4200 // 满电电压 (mV)
/*
    空电电压 (mV)
*/
// #define BATTERY_EMPTY_VOLTAGE ((u16)2900)
#define BATTERY_EMPTY_VOLTAGE ((u16)3000)
/*
    低电量警告电压 (mV)
*/
#define BATTERY_LOW_WARNING_VOLTAGE ((u16)3200)

// ADC相关参数 (电池检测使用内部2.0V参考电压，VDD 1/5分压)
#define BATTERY_ADC_REF_VOLTAGE_MV 2000 // 内部参考电压 2.0V
#define BATTERY_VOLTAGE_DIVIDER 5       // VDD经过1/5分压后输入ADC

// 计算ADC值对应的电池电压 (单位：mV)
// ADC值 -> 实际电池电压 = ADC值 * (参考电压/4096) * 分压比 * 1000
#define ADC_TO_BATTERY_VOLTAGE_MV(adc_val) \
    (((u32)(adc_val) * BATTERY_ADC_REF_VOLTAGE_MV * BATTERY_VOLTAGE_DIVIDER) / 4096)

#define VOLTAGE_HISTORY_SIZE 10
// #define VOLTAGE_HISTORY_SIZE 1
// 检测电池电压的周期，单位：ms
#define BATTERY_VOLTAGE_UPDATE_PERIOD ((u16)4000)
// 每隔多久将采集的电池电压放入缓存中，单位：ms
// #define BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER ((u32)15 * 1000)
// #define BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER ((u32)4 * 1000)
#define BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER ((u32)100)
// 每隔多久将缓存中的电池电压提取出来，单位：ms
#define BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER_EXTRACT \
    (BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER * VOLTAGE_HISTORY_SIZE)

/*
    每隔多久更新一次电池电量百分比，每次变化范围：±1

    假设客户用8节2000mah的18650电池，
    8节并联，总电池容量：16000mah -> 16 ah
    假设是通过电源管理ic的典型放电电流 2.4 A 进行放电，
    放电时间： 16 / 2.4 约为: 6.667 h

    假设 3.0V 关机，对应电池剩余电量的5%左右（可以忽略不计）

    那么最快也要 6.667 h 才会放完电
    将这个时间划为100份，每份时间对应一个百分比

    6.667 h / 100 == 0.0667h
                  约等于 240 S

    那么 每隔 240 S，才变化一次电池电量百分比，每次变化范围：±1
*/
// #define BATTERY_PERCENT_UPDATE_PERIOD ((u16)240 * 1000)
#define BATTERY_PERCENT_UPDATE_PERIOD ((u16)30 * 1000)

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

extern volatile u16 avg_voltage_mv; // @attention 在当前.c文件外调用时，慎用

// void send_low_battery_timer_callback(void);

// 计算电池电压对应的百分比 (0-100)
u8 get_battery_percentage_by_voltage(u16 voltage_mv);
u16 get_battery_voltage_by_adc(u16 adc_val); // 通过AD值计算电池电压 (单位：mV)

// void battery_monitor_init_by_adc_val(u16 adc_val);
void battery_monitor_refresh_by_adc_val(u16 adc_val);
void battery_monitor_handle(void);

void bat_vol_update_timer_callback(void);
void bat_vol_buff_add_timer_callback(void);
void bat_vol_buff_get_avg_timer_callback(void);

void send_low_bat_timer_callback(void); // 控制发送低电量的周期

void battery_voltage_update_by_isr(void);
void bat_vol_history_buff_init(u16 voltage_mv);

void bat_percent_update_time_add(void);
void bat_percent_update_time_reset(void);
u8 bat_percent_update_time_is_comes(void);

#if USER_DEBUG_ENABLE
// void user_test_init_by_voltage_mv(u16 test_voltage_mv);
#endif

#endif // __BATTERY_MONITOR_H__