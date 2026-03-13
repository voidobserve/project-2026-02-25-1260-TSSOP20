#include "led.h"
#include "pwm.h"
#include "user_test.h"

volatile led_ctl_t led_ctl;

void led_init(void)
{
    // 100% 电量指示灯
    P1_MD0 &= ~GPIO_P13_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P13_MODE_SEL(0x01); // 输出
    FOUT_S13 = GPIO_FOUT_AF_FUNC;
    P13 = 0;

    // 75% 电量指示灯
    P0_MD1 &= ~GPIO_P06_MODE_SEL(0x03);
    P0_MD1 |= GPIO_P06_MODE_SEL(0x01);
    FOUT_S06 = GPIO_FOUT_AF_FUNC;
    P06 = 0; // 不点亮指示灯

    // 50% 电量指示灯
    P0_MD1 &= ~GPIO_P05_MODE_SEL(0x03);
    P0_MD1 |= GPIO_P05_MODE_SEL(0x01);
    FOUT_S05 = GPIO_FOUT_AF_FUNC;
    P05 = 0; // 不点亮指示灯

    // 25% 电量指示灯
    P0_MD0 &= ~GPIO_P00_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P00_MODE_SEL(0x01);
    FOUT_S00 = GPIO_FOUT_AF_FUNC;
    P00 = 0; // 不点亮指示灯
}

void led_ctl_init(void)
{
    led_ctl.status = LED_STATUS_OFF;
    led_ctl.cnt = 0;
    led_ctl.cur_period_val = 0;
    led_ctl.dest_period_val = STRM0_PERIOD_30_PERCENT_VAL;
}

void led_status_switch(void)
{
    // printf("led_ctl.status == %u\n", (u16)led_ctl.status);
    switch (led_ctl.status)
    {
    case LED_STATUS_OFF:
        led_ctl.status = LED_STATUS_YELLOW;
        LED_YELLOW_ON();
        break;
    case LED_STATUS_YELLOW:
        led_ctl.status = LED_STATUS_WHITE;
        LED_YELLOW_OFF();
        LED_WHITE_ON();
        break;
    case LED_STATUS_WHITE:
        led_ctl.status = LED_STATUS_WHITE_YELLOW;
        LED_YELLOW_ON();
        break;
    case LED_STATUS_WHITE_YELLOW:
        led_ctl.status = LED_STATUS_RED_BLUE_FLASH;
        LED_YELLOW_OFF();
        LED_WHITE_OFF();
        break;
    case LED_STATUS_RED_BLUE_FLASH:
        led_ctl.status = LED_STATUS_OFF;
        P01 = 1;
        P02 = 1;
        break;
    }
} 

// 红灯、蓝灯闪烁的动画效果，由定时器调用
void led_red_blue_flash_1ms_isr(void)
{
    // USER_TO_DO 只实现了简单的闪烁功能，还不是跟客户一样的闪烁功能
    static volatile u16 cnt = 0;
    static volatile u8 dir = 0;

    if (LED_STATUS_RED_BLUE_FLASH == led_ctl.status)
    {
        cnt++;
        if (cnt >= 300)
        {
            cnt = 0;
            if (0 == dir)
            {
                P01 = 1;
                P02 = 1;

                dir = 1;
            }
            else
            {
                P01 = 0;
                P02 = 0;

                dir = 0;
            }
        }
    }
    else
    {
        cnt = 0;
    }
}

void led_slow_start_isr(void)
{
    if (led_ctl.cur_period_val >= led_ctl.dest_period_val)
    {
        return;
    }

    led_ctl.cnt++;

    // USER_TO_DO
    if (led_ctl.cnt < YELLOW_SLOW_START_ADJUST_TIME)
    {
        return;
    }

    led_ctl.cnt = 0;
    led_ctl.cur_period_val++;
    pwm_set_channel_0_duty(led_ctl.cur_period_val);
}
