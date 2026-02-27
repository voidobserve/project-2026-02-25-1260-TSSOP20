#ifndef __PWM_H__
#define __PWM_H__

#include "include.h"

// USER_TO_DO 
#define STRM0_PSC_VAL_CONFIG // 由用户设置，预分频，填入的值范围：0~254，对应1~255分频
#define STMR0_PEROID_VAL (SYSCLK / 128 / 200)
#define STMR1_PEROID_VAL (SYSCLK / 128 / 200)

// #define MAX_PWM_DUTY (STMR_ALL_PERIOD_VAL + 1) // 100%占空比   (SYSCLK 4800 0000 /  8000  == 6000)

// enum
// {
//     PWM_DUTY_100_PERCENT = MAX_PWM_DUTY,                       // 100%占空比
//     PWM_DUTY_80_PERCENT = (u16)((u32)MAX_PWM_DUTY * 80 / 100), // 80%
//     PWM_DUTY_75_PERCENT = (u16)((u32)MAX_PWM_DUTY * 75 / 100), // 75%占空比
//     PWM_DUTY_50_PERCENT = (u16)((u32)MAX_PWM_DUTY * 50 / 100), // 50%占空比
//     PWM_DUTY_30_PERCENT = (u16)((u32)MAX_PWM_DUTY * 30 / 100), // 30%占空比
//     PWM_DUTY_25_PERCENT = (u16)((u32)MAX_PWM_DUTY * 25 / 100), // 25%占空比
// };

void pwm_init(void);

#endif