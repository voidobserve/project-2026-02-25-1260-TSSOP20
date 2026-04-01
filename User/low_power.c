#include "low_power.h"

#define LOW_POWER_ENTER_TIME_WHEN_POWER_OFF ((u16)2000) // 关机后，多久进入低功耗，单位：ms

static volatile is_low_power_enter_enable = 0;

// 判断进入低功耗的条件是否成立
u8 is_low_power_condition_establish(void)
{
    // 蓝牙ic正在工作、灯光打开、正在充电，返回0，表示不进入低功耗
    return !(ble_ic.is_working || led_ctl.status != LED_STATUS_OFF || is_in_charging);
}

// 进入低功耗的条件成立，则累加时间，满足时间后进入低功耗
void low_power_enter_timer_callback(void)
{
    static u16 cnt = 0;
    if (is_low_power_condition_establish())
    {
        cnt++;
        if (cnt >= LOW_POWER_ENTER_TIME_WHEN_POWER_OFF)
        {
            cnt = 0;
            is_low_power_enter_enable = 1;
        }
    }
    else
    {
        cnt = 0;
        is_low_power_enter_enable = 0;
    }
}

void low_power_in(void)
{
    __DisableIRQ(TMR1_IRQn);  // 禁止 定时器 1 中断
    __DisableIRQ(TMR0_IRQn);  // 禁止 定时器 0 中断
    __DisableIRQ(ADC_IRQn);   // 禁止 ADC 中断
    __DisableIRQ(UART0_IRQn); // 禁止 串口 0 中断

    TMR1_CONL &= ~(TMR_MODE_SEL(0x1)); // 不使能计数
    TMR0_CONL &= ~(TMR_MODE_SEL(0x1)); // 不使能计数

    // 关闭PWM
    P30 = 1; // 不点亮 黄灯
    FOUT_S30 = GPIO_FOUT_AF_FUNC;
    P27 = 1; // 不点亮 白灯
    FOUT_S27 = GPIO_FOUT_AF_FUNC;
    STMR_CNTEN &= ~STMR_0_CNT_EN(0x1); // 不使能 STMR 0
    STMR_PWMEN &= ~STMR_0_PWM_EN(0x1); // 不使能 PWM输出
    STMR_CNTEN &= ~STMR_1_CNT_EN(0x1); // 不使能 STMR 1
    STMR_PWMEN &= ~STMR_1_PWM_EN(0x1); // 不使能 PWM输出

    ADC_CFG0 &= ~(ADC_EN(0x01)); // 关闭ADC

    // 电池电量指示灯、红灯和蓝灯配置：

    // 关闭串口
    P2_MD1 |= GPIO_P25_MODE_SEL(0x03); // TX脚配置为 模拟输入
    P2_MD1 |= GPIO_P26_MODE_SEL(0x03); // RX脚配置为 模拟输入
    FOUT_S25 = GPIO_FOUT_AF_FUNC;
    FOUT_S26 = GPIO_FOUT_AF_FUNC;
    UART0_CON0 &= ~UART_EN(0x01);

    // ==========================================================
    // 配置 wake up timer
    __EnableIRQ(WUT_IRQn);         // 使能WUT中断
    IE_EA = 1;                     // 使能总中断
    TMR_ALLCON = WUT_CNT_CLR(0x1); // 清除计数值
    /*
        使用64K时钟，64分频，那么1ms计数一次，如果周期值是(1000 - 1)，那么1s产生一次中断
    */
    WUT_PRH = TMR_PERIOD_VAL_H(((((u32)64000 / 64 / 100 - 1)) >> 8) & 0xFF); // 周期值
    WUT_PRL = TMR_PERIOD_VAL_L(((((u32)64000 / 64 / 100 - 1)) >> 0) & 0xFF);
    WUT_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);                           // 使能唤醒定时器
    WUT_CONL = TMR_SOURCE_SEL(0x7) | TMR_PRESCALE_SEL(0x06) | TMR_MODE_SEL(0x1); // 选择系统时钟，64分频，计数模式

    LP_WKPND = LP_WKUP_0_PCLR(0x1);  // 清除唤醒标志位
    LP_WKCON = (LP_WKUP_0_EDG(0x0) | // 通道0高电平触发唤醒
                LP_WKUP_0_EN(0x1));  // 唤醒通道0使能

    CLK_CON0 &= ~CLK_SYSCLK_SEL(0x3);  // 系统时钟选择rc64k
    CLK_ACON0 &= ~CLK_AIP_HRC_EN(0x1); // 关闭HRC时钟

    // P17 = 1; // 用该引脚测试低功耗的时间
    // =============================================================
    // 进入Stop低功耗模式
    LP_CON |= LP_STOP_EN(0x1);
    // 退出
    // =============================================================
    // P17 = 0; // 用该引脚测试低功耗的时间
}

void low_power_out(void)
{
    // 关闭 wake up timer
    __DisableIRQ(WUT_IRQn);           // 禁止WUT中断
    WUT_CONH &= ~TMR_PRD_IRQ_EN(0x1); // 不使能计数中断
    WUT_CONL &= ~TMR_MODE_SEL(0x1);   // 不使能计数

    CLK_ACON0 |= CLK_AIP_HRC_EN(0x1); // 使能HRC时钟
    LP_WKPND |= LP_WKUP_0_PCLR(0x1);  // 清除通道0唤醒标志位
    CLK_CON0 |= CLK_SYSCLK_SEL(0x3);  // 系统时钟选择hirc_clk

#if USER_DEBUG_ENABLE
    printf("stop out\n");
#endif
}

void low_power_handle(void)
{
    u16 adc_val;

    if (is_low_power_enter_enable)
    {
        is_low_power_enter_enable = 0;
    }
    else
    {
        return;
    }
label_low_power_in:

#if USER_DEBUG_ENABLE
    printf("stop in\n");
#endif

    low_power_in();
    low_power_out();
    is_sent_low_bat_alert = 0;

    // USER_TO_DO
    // 唤醒后，检测有没有按键操作、有没有充电，有则恢复工作，没有则回到低功耗
    //
    if (0)
    {
        u8 is_back_to_low_power = 1;

        // USER_TO_DO 需要使能adc

        adc_channel_sel(ADC_CHANNEL_SEL_AD_KEY);
        delay_ms(1);
        adc_val = adc_get_val_once();
        // 如果ad值小于ad按键的阈值，说明有按键按下
        if (adc_val < AD_KEY_INDEX_MAX_VAL)
        {
            // 有按键按下
            is_back_to_low_power = 0;
        }

        adc_channel_sel(ADC_CHANNEL_SEL_SOLAR_DET);
        delay_ms(1);
        adc_val = adc_get_val_once();
        if (((u32)adc_val * 4096 / 2400 / 2) >= 4500)
        {
            // 如果检测到大于4.5V，认为有太阳能充电
            is_back_to_low_power = 0;
        }

        // USER_TO_DO 需要检查type-c充电口一侧的电压
        


        if (is_back_to_low_power)
        {
            goto label_low_power_in;
        }
    }

    user_init();
}

// wake up timer 中断服务函数
void WUT_IRQHandler(void) interrupt WUT_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(WUT_IRQn);

    // ---------------- 用户函数处理 -------------------

    // 周期中断
    if (WUT_CONH & TMR_PRD_PND(0x1))
    {
        WUT_CONH |= TMR_PRD_PND(0x1); // 清除pending
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(WUT_IRQn);
}
