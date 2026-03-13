#ifndef __CHARGE_DET_H__
#define __CHARGE_DET_H__ 

#include "include.h"

extern volatile u8 is_in_charging_by_solar_panel;
extern volatile u8 is_in_charging_by_charger;

void charge_det_init(void);
void charge_det(void);
void detect_1khz_signal_100us(void);  // 新增：检测1KHz信号的函数



#endif