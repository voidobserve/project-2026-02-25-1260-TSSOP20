#ifndef __CHARGE_DET_H__
#define __CHARGE_DET_H__ 

#include "include.h"

// 充电ic的CH1
#define CHARGE_IC_CH1 P21
#define CHARGE_IC_CH2 P22


// extern volatile u8 is_in_charging_by_solar_panel;
// extern volatile u8 is_in_charging_by_charger;
extern volatile u8 is_in_charging;// 是否正在充电

void charge_det_init(void);
void charge_det(void);
void detect_1khz_signal_100us(void);  // 新增：检测1KHz信号的函数



#endif