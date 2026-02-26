#include "pwm.h" 
 
void pwm_init(void)
{ 
    P1_MD1 &= ~GPIO_P16_MODE_SEL(0x03); // P16 14脚
    P1_MD1 |= GPIO_P16_MODE_SEL(0x01);
    P1_MD1 &= ~GPIO_P14_MODE_SEL(0x03); // P14 16脚
    P1_MD1 |= GPIO_P14_MODE_SEL(0x01);
    FOUT_S14 = GPIO_FOUT_AF_FUNC;      // AF功能输出
    FOUT_S16 = GPIO_FOUT_STMR0_PWMOUT; // stmr0_pwmout

    STMR_CNTCLR |= STMR_0_CNT_CLR(0x1); // 清空计数值 
    STMR0_PSC = STMR_PRESCALE_VAL(0x07);                        // 预分频
    STMR0_PRH = STMR_PRD_VAL_H((STMR0_PEROID_VAL >> 8) & 0xFF); // 周期值
    STMR0_PRL = STMR_PRD_VAL_L((STMR0_PEROID_VAL >> 0) & 0xFF);
    STMR0_CMPAH = STMR_CMPA_VAL_H(((0) >> 8) & 0xFF); // 比较值
    STMR0_CMPAL = STMR_CMPA_VAL_L(((0) >> 0) & 0xFF); // 比较值
    STMR_PWMVALA |= STMR_0_PWMVALA(0x1);

    STMR_CNTMD |= STMR_0_CNT_MODE(0x1); // 连续计数模式
    STMR_LOADEN |= STMR_0_LOAD_EN(0x1); // 自动装载使能
    STMR_CNTCLR |= STMR_0_CNT_CLR(0x1); //
    STMR_CNTEN |= STMR_0_CNT_EN(0x1);   // 使能
    STMR_PWMEN |= STMR_0_PWM_EN(0x1);   // PWM输出使能


    // P15 15脚 作为第2路PWM输出
    STMR_CNTCLR |= STMR_1_CNT_CLR(0x1);                         // 清空计数值
    STMR1_PSC = STMR_PRESCALE_VAL(0x07);                        // 预分频
    STMR1_PRH = STMR_PRD_VAL_H((STMR1_PEROID_VAL >> 8) & 0xFF); // 周期值
    STMR1_PRL = STMR_PRD_VAL_L((STMR1_PEROID_VAL >> 0) & 0xFF);
    STMR1_CMPAH = STMR_CMPA_VAL_H(((0) >> 8) & 0xFF); // 比较值 (清空比较值)
    STMR1_CMPAL = STMR_CMPA_VAL_L(((0) >> 0) & 0xFF); // 比较值
    STMR_PWMVALA |= STMR_1_PWMVALA(0x1);              // STMR1 PWM输出值 ( 0x1:计数CNT大于等于比较值A,PWM输出1,小于输出0 )

    STMR_CNTMD |= STMR_1_CNT_MODE(0x1); // 连续计数模式
    STMR_LOADEN |= STMR_1_LOAD_EN(0x1); // 自动装载使能
    STMR_CNTCLR |= STMR_1_CNT_CLR(0x1); //
    STMR_CNTEN |= STMR_1_CNT_EN(0x1);   // 使能
    STMR_PWMEN |= STMR_1_PWM_EN(0x1);   // PWM输出使能
 
    P1_MD1 &= ~GPIO_P15_MODE_SEL(0x03); // P15 15脚
    P1_MD1 |= GPIO_P15_MODE_SEL(0x01);  // 输出模式
    FOUT_S15 = GPIO_FOUT_STMR1_PWMOUT;  // 选择stmr1_pwmout 
}

// 设置通道0的占空比
void set_pwm_channel_0_duty(u16 channel_duty)
{
    STMR0_CMPAH = STMR_CMPA_VAL_H(((channel_duty) >> 8) & 0xFF); // 比较值
    STMR0_CMPAL = STMR_CMPA_VAL_L(((channel_duty) >> 0) & 0xFF); // 比较值
    STMR_LOADEN |= STMR_0_LOAD_EN(0x1);                          // 自动装载使能
}

// 设置通道1的占空比
void set_pwm_channel_1_duty(u16 channel_duty)
{
    STMR1_CMPAH = STMR_CMPA_VAL_H(((channel_duty) >> 8) & 0xFF); // 比较值
    STMR1_CMPAL = STMR_CMPA_VAL_L(((channel_duty) >> 0) & 0xFF); // 比较值
    STMR_LOADEN |= STMR_1_LOAD_EN(0x1);                          // 自动装载使能
}
 
 
/**
 * @brief 获取第一路PWM的运行状态
 *
 * @return u8 0--pwm关闭，1--pwm开启
 */
u8 get_pwm_channel_0_status(void)
{
    if (STMR_PWMEN & 0x01) // 如果pwm0使能
    {
        return 1;
    }
    else // 如果pwm0未使能
    {
        return 0;
    }
}

/**
 * @brief 获取第二路PWM的运行状态
 *
 * @return u8 0--pwm关闭，1--pwm开启
 */
u8 get_pwm_channel_1_status(void)
{
    if (STMR_PWMEN & (0x01 << 1)) // 如果pwm1使能
    {
        return 1;
    }
    else // 如果 pwm 未使能
    {
        return 0;
    }
}
 
