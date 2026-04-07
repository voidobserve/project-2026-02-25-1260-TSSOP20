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
#if USER_DEBUG_ENABLE
    // printf("stop in\n");
#endif

    __DisableIRQ(TMR1_IRQn);  // 禁止 定时器 1 中断
    __DisableIRQ(TMR0_IRQn);  // 禁止 定时器 0 中断
    __DisableIRQ(ADC_IRQn);   // 禁止 ADC 中断
    __DisableIRQ(UART0_IRQn); // 禁止 串口 0 中断

    TMR1_CONL &= ~(TMR_MODE_SEL(0x03)); // 不使能计数
    TMR0_CONL &= ~(TMR_MODE_SEL(0x03)); // 不使能计数

    ADC_ACON0 &= ~(ADC_CMP_EN(0x01) |  // 不使能内部比较器
                   ADC_BIAS_EN(0x01)); // 不使能内部偏置电流
    ADC_CHS0 = 0x1F;                   // adc通道 恢复为默认值
    ADC_CFG0 &= ~(ADC_EN(0x01));       // 关闭ADC

    FIN_S7 = GPIO_FIN_SEL_GND;
    UART0_CON0 &= ~UART_EN(0x01);

    // 关闭PWM
    // P30 = 1; // 不点亮 黄灯
    // FOUT_S30 = GPIO_FOUT_AF_FUNC;
    // P27 = 1; // 不点亮 白灯
    // FOUT_S27 = GPIO_FOUT_AF_FUNC;

    // STMR_LOADEN &= ~STMR_0_LOAD_EN(0x01); // 不使能自动装载
    STMR_CNTEN &= ~STMR_0_CNT_EN(0x1); // 不使能 STMR 0
    STMR_PWMEN &= ~STMR_0_PWM_EN(0x1); // 不使能 PWM输出
    // STMR_LOADEN &= ~STMR_1_LOAD_EN(0x01); // 不使能自动装载
    STMR_CNTEN &= ~STMR_1_CNT_EN(0x1); // 不使能 STMR 1
    STMR_PWMEN &= ~STMR_1_PWM_EN(0x1); // 不使能 PWM输出

    // 检测充电ic的两个引脚
    P2_MD0 |= GPIO_P22_MODE_SEL(0x03);
    P2_MD0 |= GPIO_P21_MODE_SEL(0x03);
    P2_PU &= ~(GPIO_P22_PULL_UP(0x01) | GPIO_P21_PULL_UP(0x01)); // 关闭上拉

    // 关闭串口
    P2_PU &= ~GPIO_P26_PULL_UP(0x01);
    P2_MD1 |= GPIO_P25_MODE_SEL(0x03); // TX脚配置为 模拟输入
    P2_MD1 |= GPIO_P26_MODE_SEL(0x03); // RX脚配置为 模拟输入
    FOUT_S25 = GPIO_FOUT_AF_FUNC;

    // 电池电量指示灯
    // P0_MD0 |= GPIO_P00_MODE_SEL(0x03);
    // P0_MD1 |= GPIO_P05_MODE_SEL(0x03);
    // P0_MD1 |= GPIO_P06_MODE_SEL(0x03);
    // P1_MD0 |= GPIO_P13_MODE_SEL(0x03);

    // P1_MD1 |= GPIO_P16_MODE_SEL(0x03);
    // P1_MD1 |= GPIO_P17_MODE_SEL(0x03);
    // P3_MD0 |= GPIO_P31_MODE_SEL(0x03);

    // 这个封装其他没有引出来的引脚

#if 1
    // ==========================================================

    // 配置io唤醒
    P2_MD1 &= ~GPIO_P24_MODE_SEL(0x3); // 配置 P24 为输入模式
    FIN_S10 = GPIO_FIN_SEL_P24;        // 配置 P24 为通道0输入唤醒端口

    // USER_TO_DO
    // 充电ic检测脚 ch1
    P2_MD0 &= ~GPIO_P21_MODE_SEL(0x3);
    P2_PU &= ~GPIO_P21_PULL_UP(0x01); // 关闭上拉
    FIN_S11 = GPIO_FIN_SEL_P21;       // 配置 P21 为通道1输入唤醒端口

    delay_ms(1); // 官方提供的SDK中，这里有延时操作

    /*
           配置 wake up timer
           使用64K时钟，64分频，那么1ms计数一次，如果周期值是(1000 - 1)，那么1s产生一次中断
       */
    TMR_ALLCON = WUT_CNT_CLR(0x1); // 清除计数值
    // 10ms 唤醒一次
    // WUT_PRH = TMR_PERIOD_VAL_H(((((u32)64000 / 64 / 100 - 1)) >> 8) & 0xFF); // 周期值
    // WUT_PRL = TMR_PERIOD_VAL_L(((((u32)64000 / 64 / 100 - 1)) >> 0) & 0xFF);
    // 200ms 唤醒一次
    // WUT_PRH = TMR_PERIOD_VAL_H(((((u32)64000 / 64 / 5 - 1)) >> 8) & 0xFF); // 周期值
    // WUT_PRL = TMR_PERIOD_VAL_L(((((u32)64000 / 64 / 5 - 1)) >> 0) & 0xFF);
    // 1s 唤醒一次
    WUT_PRH = TMR_PERIOD_VAL_H(((((u32)64000 / 64)) >> 8) & 0xFF); // 周期值
    WUT_PRL = TMR_PERIOD_VAL_L(((((u32)64000 / 64)) >> 0) & 0xFF);
    WUT_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);                           // 使能唤醒定时器
    WUT_CONL = TMR_SOURCE_SEL(0x7) | TMR_PRESCALE_SEL(0x06) | TMR_MODE_SEL(0x1); // 选择系统时钟，64分频，计数模式

    LP_CON &= ~LP_ISD_DIS_LP_EN(0x1);  // 使能ISD模式下低功耗功能
    LP_WKPND |= LP_WKUP_0_PCLR(0x1);   // 清除唤醒标志位
    LP_WKCON |= (LP_WKUP_0_EDG(0x01) | // 通道0 低电平 触发唤醒
                 LP_WKUP_0_EN(0x1));   // 唤醒通道0使能

    LP_WKCON &= ~LP_WKUP_1_EDG(0x01); // 通道1 高电平 触发唤醒
    // LP_WKCON |= LP_WKUP_1_EDG(0x01); // 通道1 低电平 触发唤醒
    LP_WKCON |= LP_WKUP_1_EN(0x1); // 唤醒 通道1 使能

    LP_WKPND |= LP_WKUP_2_PCLR(0x1);  // 清除唤醒标志位
    LP_WKCON |= (LP_WKUP_2_EDG(0x0) | // 通道2高电平触发唤醒
                 LP_WKUP_2_EN(0x1));  // 唤醒通道2使能

    __EnableIRQ(WUT_IRQn); // 使能WUT中断
    IE_EA = 1;             // 使能总中断
#endif

    SYS_CON6 |= SYS_MPDN_CNT(0x3);     // 关闭程序存储器供电的延迟时间配置4个系统周期
    SYS_CON7 = (SYS_EXT_SLP_CNT(0x3) | // 退出低功耗延迟的时间配置各配四个系统周期
                SYS_MTPUP_CNT(0x3) |
                SYS_OPM_LDO_CNT(0x3) |
                SYS_CLSM_LDO_CNT(0x3));
    SYS_CON8 |= SYS_LPSLP_DIS_ANA(0x1);   // 打开低功耗sleep mode一键关模拟模块功能(即关闭TK,AMP,CMP,ADC之类的模块)
    CLK_XOSC &= ~(CLK_XOSC_LOW_EN(0x1) |  // 关闭32.768KHz低速晶振
                  CLK_XOSC_HIGH_EN(0x1)); // 关闭高速晶振
    LVD_CON0 &= ~(LVD_VCC_DETE_EN(0x1) |  // 关闭VCC电源VCC电压低电检测功能
                  LVD_VDD_DETE_EN(0x1));  // 关闭1.5V数字逻辑系统工作电压VDD低电检测功能
    PMU_CON2 &= ~0x70;                    // 关闭VPTAT_ADC输出，关闭温度传感器输出VPTAT，主LDO过流档位选择50mA
    CLK_CON0 &= ~CLK_SYSCLK_SEL(0x3);     // 系统时钟选择rc64k
    FLASH_TIMEREG1 = 0x0;                 // 配置LIRC时FLASH访问速度

    PMU_CON0 &= ~0x60;                 // 关闭VDD POR模块, 关闭VBG06_REF输出
    CLK_ACON0 &= ~CLK_AIP_HRC_EN(0x1); // 关闭HRC时钟

    // P17 = 1; // 用该引脚测试低功耗的时间
    // =============================================================
    // 进入 sleep 低功耗模式
    LP_CON = (LP_IDLE_EN(0x1) |     // Idle低功耗模式使能
              LP_SLEEP_GO_EN(0x1) | // Sleep低功耗模式唤醒后继续跑后续程序
              LP_SLEEP_EN(0x1));    // 使能睡眠
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

    // 不使能唤醒通道
    LP_WKCON &= ~(LP_WKUP_0_EN(0x01) |
                  LP_WKUP_1_EN(0x01) |
                  LP_WKUP_2_EN(0x01));

    FIN_S10 = GPIO_FIN_SEL_GND;
    FIN_S11 = GPIO_FIN_SEL_GND;

    // CLK_ACON0 |= CLK_AIP_HRC_EN(0x1); // 使能HRC时钟
    // LP_WKPND |= LP_WKUP_0_PCLR(0x1);  // 清除通道0唤醒标志位
    // CLK_CON0 |= CLK_SYSCLK_SEL(0x3);  // 系统时钟选择hirc_clk

    SYS_CON8 &= ~SYS_LPSLP_DIS_ANA(0x1); // 关闭低功耗sleep mode一键关模拟模块功能(即打开TK,AMP,CMP,ADC之类的模块)
    PMU_CON0 |= 0x60;                    // 使能VDD POR模块，使能VBG06_REF输出
    LVD_CON0 |= (LVD_VCC_DETE_EN(0x1) |  // 使能VCC电源VCC电压低电检测功能
                 LVD_VDD_DETE_EN(0x1));  // 使能1.5V数字逻辑系统工作电压VDD低电检测功能
    PMU_CON2 |= 0x70;                    // 使能VPTAT_ADC输出，使能温度传感器输出VPTAT，主LDO过流档位选择100mA
    CLK_ACON0 |= CLK_AIP_HRC_EN(0x1);    // 使能HRC时钟
    // USER_TO_DO 看看这里要不要清对应的唤醒标志
    LP_WKPND |= LP_WKUP_2_PCLR(0x1); // 清除通道2唤醒标志位
    FLASH_TIMEREG1 = 0x58;           // FLASH访问速度 = 系统时钟/3
    CLK_CON0 |= CLK_SYSCLK_SEL(0x3); // 系统时钟选择hirc_clk

#if USER_DEBUG_ENABLE
    uart0_init();
    // printf("stop out\n");
#endif
}

void low_power_handle(void)
{
    u8 is_back_to_low_power = 1; // 默认从低功耗唤醒后，又返回低功耗
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

    low_power_in();
    low_power_out();

    // 唤醒后，检测有没有按键操作、有没有充电，有则恢复工作，没有则回到低功耗
    adc_pin_init();
    adc_init_when_low_pwr_out();

    adc_channel_sel(ADC_CHANNEL_SEL_AD_KEY);
    delay_ms(1);
    adc_val = adc_get_val_once();
    // 如果ad值小于ad按键的阈值，说明有按键按下
    if (adc_val < AD_KEY_INDEX_MAX_VAL)
    {
        // 有按键按下
        is_back_to_low_power = 0;
#if USER_DEBUG_ENABLE
        printf("key down\n");
#endif
    }

    adc_channel_sel(ADC_CHANNEL_SEL_SOLAR_DET);
    delay_ms(1);
    adc_val = adc_get_val_once();
    // 如果检测到大于4.5V，认为有太阳能充电
    if (((u32)adc_val * 4096 / 2400 / 2) >= 4500)
    {
        is_back_to_low_power = 0;
#if USER_DEBUG_ENABLE
        printf("charge by solar panel\n");
#endif
    }

    adc_channel_sel(ADC_CHANNEL_SEL_BAT_DET);
    delay_ms(1);
    adc_val = adc_get_val_once();
    // 从低功耗唤醒之后，需要采集一次电池对应的ad值，更新电池相关的变量
    battery_monitor_refresh_by_adc_val(adc_val);

    // USER_TO_DO 需要检查type-c充电口一侧的电压，判断有没有充电

    // USER_TO_DO 旧版的pcb，需要打开定时器1，看看充电ic有没有信号，判断是不是正在充电
    // charge_det();
    // if (is_in_charging)
    // {
    //     is_back_to_low_power = 0;
    // }
    // 如果是充电ic输出正在充电的信号，导致的唤醒
    if (LP_WKPND & LP_WKUP_1_PND(0x01))
    {
        is_back_to_low_power = 0;
#if USER_DEBUG_ENABLE
        printf("wake up by charging\n");
#endif
        LP_WKPND |= LP_WKUP_1_PCLR(0x01); // 清空唤醒标志位
    }

    if (is_back_to_low_power)
    {
#if USER_DEBUG_ENABLE
        // printf("goto low power in\n");
#endif
        goto label_low_power_in;
    }

    // 成功退出低功耗之后，清除低电量报警和低电量关机的标志位
    is_sent_low_bat_alert = 0;
    is_turn_off_by_low_bat = 0;
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
