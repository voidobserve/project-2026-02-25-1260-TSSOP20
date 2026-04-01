#include "led.h"
#include "pwm.h"
#include "user_test.h"
#include "user_include.h"
#include "string.h"

volatile led_ctl_t led_ctl;

void led_init(void)
{
    // 100% 电量指示灯
    P1_MD0 &= ~GPIO_P13_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P13_MODE_SEL(0x01); // 输出
    P13 = 0;
    FOUT_S13 = GPIO_FOUT_AF_FUNC;

    // 75% 电量指示灯
    P0_MD1 &= ~GPIO_P06_MODE_SEL(0x03);
    P0_MD1 |= GPIO_P06_MODE_SEL(0x01);
    P06 = 0; // 不点亮指示灯
    FOUT_S06 = GPIO_FOUT_AF_FUNC;

    // 50% 电量指示灯
    P0_MD1 &= ~GPIO_P05_MODE_SEL(0x03);
    P0_MD1 |= GPIO_P05_MODE_SEL(0x01);
    P05 = 0; // 不点亮指示灯
    FOUT_S05 = GPIO_FOUT_AF_FUNC;

    // 25% 电量指示灯
    P0_MD0 &= ~GPIO_P00_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P00_MODE_SEL(0x01);
    P00 = 0; // 不点亮指示灯
    FOUT_S00 = GPIO_FOUT_AF_FUNC;

    // P01 蓝灯
    P0_MD0 &= ~GPIO_P01_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P01_MODE_SEL(0x01); // 输出模式
    P01 = 1;
    FOUT_S01 = GPIO_FOUT_AF_FUNC;

    // P02 红灯
    P0_MD0 &= ~GPIO_P02_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P02_MODE_SEL(0x01); // 输出模式
    P02 = 1;
    FOUT_S02 = GPIO_FOUT_AF_FUNC;
}

void led_ctl_init(void)
{
    // led_ctl.status = LED_STATUS_OFF;
    // led_ctl.cnt = 0;
    // led_ctl.cur_pwm_duty_val = PWM_DUTY_VAL_PERCENT_0;
    // led_ctl.is_slowly_adjust_end = 0;
    // led_ctl.adjust_time_cnt = 0;
    // led_ctl.working_time = 0;

    // 目前可以用这个来代替
    memset(&led_ctl, 0, sizeof(led_ctl_t));
}

void led_yellow_on(void)
{
    pwm_set_channel_0_duty(STRM0_PERIOD_30_PERCENT_VAL);
    FOUT_S30 = GPIO_FOUT_STMR0_PWMOUT;
}

void led_yellow_off(void)
{
    P30 = 1;
    FOUT_S30 = GPIO_FOUT_AF_FUNC;
}

void led_white_on(void)
{
    pwm_set_channel_1_duty(STRM1_PERIOD_30_PERCENT_VAL);
    FOUT_S27 = GPIO_FOUT_STMR1_PWMOUT;
}

void led_white_off(void)
{
    P27 = 1;
    FOUT_S27 = GPIO_FOUT_AF_FUNC;
}

void led_status_switch(void)
{
    // printf("led_ctl.status == %u\n", (u16)led_ctl.status);
    switch (led_ctl.status)
    {
    case LED_STATUS_OFF:
        // 关灯 -> 打开黄灯
        // led_ctl.status = LED_STATUS_YELLOW;
        // led_yellow_on();
        led_status_set(LED_STATUS_YELLOW);
        break;
    case LED_STATUS_YELLOW:
        // 黄灯打开 -> 关闭黄灯，打开白灯
        // led_ctl.status = LED_STATUS_WHITE;
        // led_yellow_off();
        // led_white_on();
        led_status_set(LED_STATUS_WHITE);
        break;
    case LED_STATUS_WHITE:
        // 白灯打开 -> 打开白灯，打开黄灯
        // led_ctl.status = LED_STATUS_WHITE_YELLOW;
        // led_yellow_on();
        led_status_set(LED_STATUS_WHITE_YELLOW);
        break;
    case LED_STATUS_WHITE_YELLOW:
        // 白灯和黄灯都打开 -> 关闭黄灯和白灯，执行红灯和蓝灯闪烁的动画（红灯和蓝灯闪烁由其他函数来控制）
        // led_ctl.red_blue_flash_time_cnt = 0; // 红灯和蓝灯闪烁的时间计数清零
        // led_ctl.status = LED_STATUS_RED_BLUE_FLASH;
        // led_yellow_off();
        // led_white_off();
        led_status_set(LED_STATUS_RED_BLUE_FLASH);
        break;
    case LED_STATUS_RED_BLUE_FLASH:
        // 当前红灯和蓝灯都闪烁 -> 关闭红灯和蓝灯
        // led_ctl.status = LED_STATUS_OFF;

        // // 这里要关闭红灯和蓝灯
        // LED_RED_OFF();
        // LED_BLUE_OFF();

        // led_ctl.working_time = 0;         // 工作时间清零
        // led_ctl.cur_pwm_duty_val = 0;     // PWM占空比值清零
        // led_ctl.is_slowly_adjust_end = 0; // 表示没有慢速调节结束
        led_status_set(LED_STATUS_OFF);
        break;
    }
}

// 设置led灯的状态
void led_status_set(led_status_t status)
{

    switch (status)
    {
    case LED_STATUS_OFF: // 关灯
        led_yellow_off();
        led_white_off();
        LED_RED_OFF();
        LED_BLUE_OFF();

        led_ctl.working_time = 0;         // 工作时间清零
        led_ctl.cur_pwm_duty_val = 0;     // PWM占空比值清零
        led_ctl.is_slowly_adjust_end = 0; // 表示没有慢速调节结束
        break;
    case LED_STATUS_YELLOW:
        led_white_off();
        LED_RED_OFF();
        LED_BLUE_OFF();

        led_yellow_on(); // 只亮黄灯
        break;
    case LED_STATUS_WHITE:
        led_yellow_off();
        LED_RED_OFF();
        LED_BLUE_OFF();

        led_white_on(); // 只亮白灯
        break;

    case LED_STATUS_WHITE_YELLOW:
        LED_RED_OFF();
        LED_BLUE_OFF();

        // 只亮黄灯和白灯
        led_white_on();
        led_yellow_on();
        break;

    case LED_STATUS_RED_BLUE_FLASH: // 红蓝闪烁
        led_yellow_off();
        led_white_off();

        led_ctl.red_blue_flash_time_cnt = 0;
        break;
    }

    led_ctl.status = status; // 需要最后再给状态赋值，否则会在定时器中断先执行了相关的操作
}

/**
 * @brief 红灯、蓝灯闪烁的动画效果，由定时器调用
 *
 * @attention 进入该动画前，要先给 led_ctl.red_blue_flash_time_cnt 清零
 *
 */
void led_red_blue_flash_1ms_isr(void)
{
    u16 time_in_cycle; // 后续再赋值

    if (led_ctl.status != LED_STATUS_RED_BLUE_FLASH)
    {
        return;
    }

    led_ctl.red_blue_flash_time_cnt++;

    // 小动画1：红灯和蓝灯的波形错位400ms，最开始蓝灯先点亮，低电平200ms，高电平600ms，持续 7.8s
    if (led_ctl.red_blue_flash_time_cnt <= (u32)7800) // 7.8秒 = 7800毫秒
    {
        time_in_cycle = led_ctl.red_blue_flash_time_cnt % 800; // 800ms周期 (200ms低+600ms高)

        // 蓝灯先亮，红灯延迟400ms亮
        if (time_in_cycle < 200) // 前200ms，蓝灯低电平（亮），红灯高电平（灭）
        {
            LED_BLUE_ON(); // 蓝灯亮
            LED_RED_OFF(); // 红灯灭
        }
        else if (time_in_cycle < 400) // 接下来200ms，红蓝灯都灭
        {
            LED_BLUE_OFF();
            LED_RED_OFF();
        }
        else if (time_in_cycle < 600) // 再接下来200ms，红灯亮（低电平），蓝灯灭（高电平）
        {
            LED_BLUE_OFF();
            LED_RED_ON();
        }
        else // 最后200ms，红蓝灯都灭（高电平）
        {
            LED_BLUE_OFF();
            LED_RED_OFF();
        }
    }
    // 小动画2：红灯和蓝灯波形不错位，低电平230ms，高电平220ms，持续 4.26s
    else if (led_ctl.red_blue_flash_time_cnt <= (u32)7800 + 4260) // 7.8s + 4.26s = 12060ms
    {
        time_in_cycle = (led_ctl.red_blue_flash_time_cnt - 7800) % ((u16)230 + 220); // 450ms周期 (230ms低+220ms高)

        if (time_in_cycle < 230) // 低电平230ms，灯亮（低电平）
        {
            LED_RED_ON();  // 红灯亮
            LED_BLUE_ON(); // 蓝灯亮
        }
        else // 高电平220ms，灯灭（高电平）
        {
            LED_RED_OFF();  // 红灯灭
            LED_BLUE_OFF(); // 蓝灯灭
        }
    }
    // 小动画3：红灯和蓝灯都熄灭（引脚输出高电平）230ms，之后蓝灯先点亮40ms，再熄灭60ms，重复3次，
    // 到红灯点亮40ms，熄灭60ms，重复3次，再到蓝灯点亮40ms...整个动画持续6.17s 最后红灯和蓝灯都熄灭
    else if (led_ctl.red_blue_flash_time_cnt <= (u32)7800 + 4260 + 6170) // 12060ms + 6.17s = 18230ms
    {
        // 前230ms，红灯和蓝灯都熄灭
        if (led_ctl.red_blue_flash_time_cnt < (u32)7800 + 4260 + 230)
        {
            LED_RED_OFF();
            LED_BLUE_OFF();
            return;
        }

        // (40 + 60) * 3 * 2，(点亮40ms+熄灭60ms)*重复3次 * 2（红灯和蓝灯）
        time_in_cycle = (led_ctl.red_blue_flash_time_cnt - 7800 - 4260 - 230) % (((u16)40 + 60) * 3 * 2);
        // 蓝灯循环：点亮40ms，熄灭60ms，重复3次
        if (time_in_cycle < ((u16)40 + 60) * 3)
        {
            LED_RED_OFF();
            if ((time_in_cycle % (40 + 60)) < 40)
            {
                LED_BLUE_ON();
            }
            else
            {
                LED_BLUE_OFF();
            }
        }
        // 红灯循环：点亮40ms，熄灭60ms，重复3次
        else
        {
            LED_BLUE_OFF();
            if ((time_in_cycle % (40 + 60)) < 40)
            {
                LED_RED_ON();
            }
            else
            {
                LED_RED_OFF();
            }
        }
    }
    else
    {
        LED_RED_OFF();
        LED_BLUE_OFF();
        led_ctl.red_blue_flash_time_cnt = 0; // 计时清零，让动画重新开始跑
    }
}

// 黄灯、白灯、黄白灯缓慢调节的动画效果
void led_slow_adjust_isr(void)
{
    if (led_ctl.status != LED_STATUS_OFF && led_ctl.is_slowly_adjust_end != 1)
    {
        // 如果灯光正在工作，并且缓慢调节没有结束
        led_ctl.working_time++;
    }
    else
    {
        return;
    }

    /*
        假设每1ms调节一次占空比
        总共 480s，前180s不调节，后面300s缓慢调节占空比至 30%

        现在 30% 对应的占空比值为 STRM0_PERIOD_30_PERCENT_VAL 和 STRM1_PERIOD_30_PERCENT_VAL
        300s调节时间中，每调节单位1的占空比值要经过 PWM_DUTY_SLOW_ADJUST_UNIT 的时间
    */
    if (led_ctl.working_time <= (u32)180 * 1000)
    {
        // 开灯的前180s不调节
        return;
    }

    // 开灯180s之后，每 xx ms调节1单位的占空比值
    led_ctl.adjust_time_cnt++;
    if (led_ctl.adjust_time_cnt >= PWM_DUTY_SLOW_ADJUST_UNIT)
    {
        led_ctl.adjust_time_cnt = 0;
        if (led_ctl.cur_pwm_duty_val < PWM_DUTY_VAL_PERCENT_X(30))
        {
            led_ctl.cur_pwm_duty_val++;
            if (led_ctl.status == LED_STATUS_WHITE ||
                led_ctl.status == LED_STATUS_WHITE_YELLOW)
            {
                pwm_set_channel_0_duty(led_ctl.cur_pwm_duty_val);
            }

            if (led_ctl.status == LED_STATUS_YELLOW ||
                led_ctl.status == LED_STATUS_WHITE_YELLOW)
            {

                pwm_set_channel_1_duty(led_ctl.cur_pwm_duty_val);
            }

            if (led_ctl.cur_pwm_duty_val >= PWM_DUTY_VAL_PERCENT_X(30))
            {
                led_ctl.is_slowly_adjust_end = 1; // 表示缓慢调节结束
            }
        }
    }
}

volatile led_bat_level_sta_t led_bat_level_sta = LED_BAT_LEVEL_STA_IDLE;
volatile u8 led_charge_anim_phase = 0; // 充电开始的动画阶段，0：灯光全灭，1：开始点亮第一个灯
volatile u16 led_charge_anim_cnt = 0;

// 充电或放电时，电池电量指示灯的动画
// 外部参数： 全局变量 bat_percent ，电池电量百分比
void led_bat_instruction_timer_callback(void)
{
    if (led_bat_level_sta == LED_BAT_LEVEL_STA_IDLE)
    {
        LED_100_PERCENT_OFF();
        LED_75_PERCENT_OFF();
        LED_50_PERCENT_OFF();
        LED_25_PERCENT_OFF();
    }
    else if (led_bat_level_sta == LED_BAT_LEVEL_STA_CHARGE_BEGIN)
    {
        LED_100_PERCENT_OFF();
        LED_75_PERCENT_OFF();
        LED_50_PERCENT_OFF();
        LED_25_PERCENT_OFF();
        led_charge_anim_phase = 0;
        led_charge_anim_cnt = 0;
        led_bat_level_sta = LED_BAT_LEVEL_STA_CHARGE_BEGIN_ANIM;
    }
    else if (led_bat_level_sta == LED_BAT_LEVEL_STA_CHARGE_BEGIN_ANIM)
    {
        led_charge_anim_cnt++;
        if (led_charge_anim_cnt >= 500)
        {
            led_charge_anim_cnt = 0;

            if (led_charge_anim_phase == 0 &&
                bat_percent >= 25)
            {
                LED_25_PERCENT_ON();
                led_charge_anim_phase = 1;
            }
            else if (led_charge_anim_phase == 1 &&
                     bat_percent >= 50)
            {
                LED_50_PERCENT_ON();
                led_charge_anim_phase = 2;
            }
            else if (led_charge_anim_phase == 2 &&
                     bat_percent >= 75)
            {
                LED_75_PERCENT_ON();
                led_charge_anim_phase = 3;
            }
            else
            {
                led_charge_anim_cnt = 0;
                led_bat_level_sta = LED_BAT_LEVEL_STA_CHARGING;
            }
        }
    }
    else if (led_bat_level_sta == LED_BAT_LEVEL_STA_CHARGING)
    {
        // 正在充电，让当前电量对应的指示灯闪烁（只闪烁一个灯）
        // led_charge_anim_cnt == 0，说明刚进入这个阶段，也需要对应的指示灯状态翻转一次
        if (led_charge_anim_cnt >= 500 || led_charge_anim_cnt == 0)
        {
            led_charge_anim_cnt = 0;
            switch (led_charge_anim_phase)
            {
            case 0:
                LED_25_PERCENT_TOGGLE();
                break;
            case 1:
                LED_50_PERCENT_TOGGLE();
                break;
            case 2:
                LED_75_PERCENT_TOGGLE();
                break;
            case 3:
                LED_100_PERCENT_TOGGLE();
                break;
            }
        }

        led_charge_anim_cnt++;

        // 在充电过程中，电池电量增加，需要及时更新状态

        // 如果电池电量已经到100%，并且充电IC已经停止工作，则说明充电结束
        // if (bat_percent >= 100 && is_charging_ic_stop)
        if (is_charging_ic_stop)
        {
            led_bat_level_sta = LED_BAT_LEVEL_STA_CHARGE_END;
            printf("detect charge end\n");
        }
        else if (bat_percent >= 75)
        {
            led_charge_anim_phase = 3;
            LED_75_PERCENT_ON(); //
        }
        else if (bat_percent >= 50)
        {
            led_charge_anim_phase = 2;
            LED_50_PERCENT_ON(); //
        }
        else if (bat_percent >= 25)
        {
            led_charge_anim_phase = 1;
            LED_25_PERCENT_ON(); //
        }
    }
    else if (led_bat_level_sta == LED_BAT_LEVEL_STA_CHARGE_END)
    {
        LED_100_PERCENT_ON();
        LED_75_PERCENT_ON();
        LED_50_PERCENT_ON();
        LED_25_PERCENT_ON();
    }
    else if (led_bat_level_sta == LED_BAT_LEVEL_STA_DISCHARGE)
    {
        // 放电中
        if (bat_percent >= 75)
        {
            LED_100_PERCENT_ON();
        }

        if (bat_percent >= 50)
        {
            LED_75_PERCENT_ON();
        }

        if (bat_percent >= 25)
        {
            LED_50_PERCENT_ON();
        }

        if (bat_percent >= 0)
        {
            LED_25_PERCENT_ON();
        }
    }
}
