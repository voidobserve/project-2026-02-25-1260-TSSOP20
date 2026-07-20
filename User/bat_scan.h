#ifndef __BAT_SCAN_H__
#define __BAT_SCAN_H__

#include "typedef.h"
#include "user_config.h"

// 电池电量更新模块的状态
enum
{
	BAT_VOL_UPDATE_STA_IDLE,	  // 空闲
	BAT_VOL_UPDATE_STA_CAPTURING, // 正在采集
	BAT_VOL_UPDATE_STA_COMPLETED, // 采集完成
}; // battery voltage update status
typedef u8 bat_vol_update_sta_t;

// 电池模型参数
#define BATTERY_FULL_VOLTAGE 4200 // 满电电压 (mV)
/*
	空电电压 (mV)
*/
// #define BATTERY_EMPTY_VOLTAGE ((u16)2900)
#define BATTERY_EMPTY_VOLTAGE ((u16)3000)
/*
	低电量提示电压 (mV)
*/
#define BATTERY_LOW_WARNING_VOLTAGE ((u16)3250)

// ADC相关参数 (电池检测使用内部2.0V参考电压，VDD 1/5分压)
#define BATTERY_ADC_REF_VOLTAGE_MV 2000 // 内部参考电压 2.0V
#define BATTERY_VOLTAGE_DIVIDER 5		// VDD经过1/5分压后输入ADC

// =================================================================
// =================================================================
// 计算ADC值对应的电池电压 (单位：mV)
// ADC值 -> 实际电池电压 = ADC值 * (参考电压/4096) * 分压比 * 1000
#define ADC_TO_BATTERY_VOLTAGE_MV(adc_val) \
	(((u32)(adc_val) * BATTERY_ADC_REF_VOLTAGE_MV * BATTERY_VOLTAGE_DIVIDER) / 4096)

// 检测多长时间的最高电池电压，单位：ms
#define BATTERY_VOLTAGE_UPDATE_PERIOD ((u16)4000)

// 电池电压滑动平局数据的大小：
#define VOLTAGE_HISTORY_SIZE 100
// 每隔多久将采集的电池电压放入缓存中，单位：ms
#define BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER ((u32)100)
// 每隔多久将缓存中的电池电压提取出来，单位：ms
#if 0
#define BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER_EXTRACT \
	((u32)BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER * VOLTAGE_HISTORY_SIZE)
#endif
#define BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER_EXTRACT \
	((u32)1000)

extern volatile u16 avg_voltage_mv;	   // @attention 在当前.c文件外调用时，慎用
extern volatile u16 voltage_mv_global; // 只在测试时引到外部，打印

u16 bat_vol_history_buff_get_avg(void);

void battery_voltage_update_by_isr(void);
void bat_vol_history_buff_init(u16 voltage_mv);

// 不使用电池电量百分比
// 计算电池电压对应的百分比 (0-100)
// u8 get_battery_percentage_by_voltage(u16 voltage_mv);
u16 get_battery_voltage_by_adc(u16 adc_val); // 通过AD值计算电池电压 (单位：mV)

void bat_vol_update_timer_callback(void);
void bat_vol_history_buff_add_timer_callback(void);

void bat_get_avg_vol_period_add(void);

void bat_scan(void);

#endif
