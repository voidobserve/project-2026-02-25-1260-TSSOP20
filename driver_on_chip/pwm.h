#ifndef __PWM_H__
#define __PWM_H__

#include "include.h"

//
#define STRM0_PSC_VAL_CONFIG (8 - 1) // 由用户设置，预分频，填入的值范围：0~254，对应1~255分频
#define STMR0_PERIOD_VAL (SYSCLK / (STRM0_PSC_VAL_CONFIG + 1) / 200)
#define STRM0_PERIOD_30_PERCENT_VAL ((u32)STMR0_PERIOD_VAL * 30 / 100)
#define STMR0_PERIOD_0_PERCENT_VAL ((u32)STMR0_PERIOD_VAL * 0 / 100)

#define STRM1_PSC_VAL_CONFIG (8 - 1)
#define STMR1_PERIOD_VAL (SYSCLK / (STRM1_PSC_VAL_CONFIG + 1) / 200)
#define STRM1_PERIOD_30_PERCENT_VAL ((u32)STMR1_PERIOD_VAL * 30 / 100)
#define STMR1_PERIOD_0_PERCENT_VAL ((u32)STMR1_PERIOD_VAL * 0 / 100)

// stmr0 和 stmr1 的占空比值应该一样  
enum
{
    PWM_DUTY_VAL_PERCENT_30 = ((u16)((u32)STMR0_PERIOD_VAL * 30 / 100)), // 30%占空比
    PWM_DUTY_VAL_PERCENT_0 = 0, // 0%占空比
};
#define PWM_DUTY_VAL_PERCENT_X(X) ((u16)((u32)STMR0_PERIOD_VAL * X / 100))
 

void pwm_init(void);
void pwm_set_channel_0_duty(u16 channel_duty);
void pwm_set_channel_1_duty(u16 channel_duty);

#endif