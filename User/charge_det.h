#ifndef __CHARGE_DET_H__
#define __CHARGE_DET_H__

#include "include.h"

// 充电ic的CH1
#define CHARGE_IC_CH1 P21
#define CHARGE_IC_CH2 P22

// ADC 相关参数
// type-c 检测时，adc使用的参考电压
#define TYPE_C_DET_ADC_REF_VOL_MV ((u16)2400)
/*
	type-c 检测，检测脚采集到的电压与实际的电压之前的比例，
	实际电压/采集到的电压

	目前是 type-c 经过 1/2 分压后，再到检测脚，所以是 2
*/
#define TYPE_C_DET_ADC_VOL_RATIO ((u16)2)
/*
	将 type-c 脚采集到的ad值，转换成实际的电压值（单位：mV）
	实际的电压 == ad值 * (参考电压/4096) * 分压系数

	@NOTE 由于 adc 用的是2.4V参考电压，这里最大只能计算出 4800 mV 的电压值
*/ 
#define TYPE_C_DET_ADC_VAL_TO_VOL(adc_val)         \
	((u16)(((u32)adc_val) * TYPE_C_DET_ADC_REF_VOL_MV * \
		   TYPE_C_DET_ADC_VOL_RATIO / 4096))

extern volatile u8 is_in_charging_by_solar_panel; // 是否正在通过太阳能充电
extern volatile u8 is_in_charging_by_charger;	  // 是否正在通过充电IC充电

extern volatile u8 is_in_charging;			   // 是否正在充电
extern volatile u8 is_charging_ic_stop;		   // 充电ic是否停止充电
extern volatile u8 is_in_discharging;		   // 是否正在放电
extern volatile u8 is_detect_discharge_signal; // 是否检测到了连续的放电信号

void charge_det_init(void);
void charge_det(void);
void detect_1khz_signal_100us(void);		   // 新增：检测1KHz信号的函数
void detect_discharge_1khz_signal_100us(void); // 检测放电的1KHz信号
void charge_det_time_add(void);

#endif