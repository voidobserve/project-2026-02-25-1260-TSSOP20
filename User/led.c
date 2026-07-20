#include "led.h"
#include "pwm.h"
#include "user_test.h"
#include "user_include.h"
#include <string.h>
#include "bat_scan.h"
#include "battery_monitor.h"

volatile led_ctl_t led_ctl;

// 电池指示灯跳级向下的去抖时间（ms），发生跳级时，每隔该时间降一档
#define LED_BAT_JUMP_DOWN_DEBOUNCE_MS ((u32)10 * 60 * 1000)

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

#if 0
/**
 * @brief 初始化电池电量指示灯对应的状态
 *      只在第一次上电后调用
 *
 */
void led_bat_lev_sta_init(u16 voltage_mv)
{
    static u8 is_initialized = 0;
    if (is_initialized)
    {
        return;
    }

    is_initialized = 1;

    // 直接根据黄白灯都点亮对应的电池电压来划分
    if (voltage_mv >= BAT_WY_4LED_VOLTAGE)
    {
        led_bat_lev_sta = LED_BAT_LEV_4;
    }
    else if (voltage_mv >= BAT_WY_3LED_VOLTAGE)
    {
        led_bat_lev_sta = LED_BAT_LEV_3;
    }
    else if (voltage_mv >= BAT_WY_2LED_VOLTAGE)
    {
        led_bat_lev_sta = LED_BAT_LEV_2;
    }
    // else if (voltage_mv >=
    //          (BAT_WY_LOW_WARN_VOLTAGE + BAT_WY_DEAD_ZONE_VOLTAGE))
    else
    {
        led_bat_lev_sta = LED_BAT_LEV_1;
    }
}
#endif

void led_yellow_on(void)
{
    // pwm_set_channel_0_duty(STMR0_PERIOD_30_PERCENT_VAL);
    pwm_set_channel_0_duty(STMR0_PERIOD_0_PERCENT_VAL);
    // led_ctl.cur_pwm_duty_val = STMR0_PERIOD_0_PERCENT_VAL;
    FOUT_S30 = GPIO_FOUT_STMR0_PWMOUT;
}

#if 0
// 只由 led_resume() 调用
void __led_yellow_resume__(void)
{
    pwm_set_channel_0_duty(led_ctl.cur_pwm_duty_val);
    FOUT_S30 = GPIO_FOUT_STMR0_PWMOUT;
}
#endif

void led_yellow_off(void)
{
    P30 = 1;
    FOUT_S30 = GPIO_FOUT_AF_FUNC;
}

void led_white_on(void)
{
    // pwm_set_channel_1_duty(STMR1_PERIOD_30_PERCENT_VAL);
    pwm_set_channel_1_duty(STMR1_PERIOD_0_PERCENT_VAL); //
    // led_ctl.cur_pwm_duty_val = STMR1_PERIOD_0_PERCENT_VAL;
    FOUT_S27 = GPIO_FOUT_STMR1_PWMOUT;
}

#if 0
// 只由 led_resume() 调用
void __led_white_resume__(void)
{
    pwm_set_channel_1_duty(led_ctl.cur_pwm_duty_val);
    FOUT_S27 = GPIO_FOUT_STMR1_PWMOUT;
}
#endif

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
    // 每次切换状态时，都清空工作时间
    led_ctl.working_time = 0;     // 工作时间清零
    led_ctl.cur_pwm_duty_val = 0; // PWM占空比值清零（表示当前灯光为最亮）
    // led_ctl.is_slowly_adjust_end = 0; // 表示没有慢速调节结束
    led_ctl.adjust_time_cnt = 0;

    switch (status)
    {
    case LED_STATUS_OFF: // 关灯
        led_yellow_off();
        led_white_off();
        LED_RED_OFF();
        LED_BLUE_OFF();
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
    if ((led_ctl.status == LED_STATUS_YELLOW ||
         led_ctl.status == LED_STATUS_WHITE ||
         led_ctl.status == LED_STATUS_WHITE_YELLOW))
    {
        /*
            黄灯、白灯、黄白灯模式下，
            并且缓慢调节灯光的操作没有结束，
            记录灯光工作时间，进行缓慢调节
        */
        if (led_ctl.working_time < ((u32)-1)) // 防止计数溢出
        {
            led_ctl.working_time++;
        }
    }
    else
    {
        // 不在黄灯、白灯、黄白灯模式，或者缓慢调节已经结束
        return;
    }

    if (is_in_charging)
    {
        // 正在充电，但是占空比没有调节至目标占空比
        led_ctl.dest_pwm_duty_val = PWM_DUTY_VAL_PERCENT_X(PWM_DEST_DUTY_PERCENT_DURING_CHARGING);

        // 每 xx ms调节1单位的占空比值
        led_ctl.adjust_time_cnt++;
        if (led_ctl.adjust_time_cnt >= PWM_DUTY_SLOW_ADJUST_UNIT_DURING_CHARGING)
        {
            led_ctl.adjust_time_cnt = 0;
            if (led_ctl.cur_pwm_duty_val < led_ctl.dest_pwm_duty_val)
            {
                led_ctl.cur_pwm_duty_val++;
                if (led_ctl.status == LED_STATUS_YELLOW ||
                    led_ctl.status == LED_STATUS_WHITE_YELLOW)
                {
                    pwm_set_channel_0_duty(led_ctl.cur_pwm_duty_val);
                }

                if (led_ctl.status == LED_STATUS_WHITE ||
                    led_ctl.status == LED_STATUS_WHITE_YELLOW)
                {
                    pwm_set_channel_1_duty(led_ctl.cur_pwm_duty_val);
                }
            }
        }
    }
    else
    {
        // 没有在充电

        // 180s 之后，每 xx ms调节1单位的占空比值
        led_ctl.dest_pwm_duty_val = PWM_DUTY_VAL_PERCENT_X(PWM_DEST_DUTY_PERCENT);
        led_ctl.adjust_time_cnt++;

        // 占空比越小，灯光亮度越高
        if (led_ctl.cur_pwm_duty_val < led_ctl.dest_pwm_duty_val)
        {
            // 如果当前占空比 小于 目标占空比（当前灯光亮度大于目标亮度）
            // 要按照 正常工作 的缓慢速度进行调节

#if 1 // REVIEW 测试时屏蔽
            if (led_ctl.working_time <= (u32)180 * 1000)
            {
                // 开灯的前180s不调节
                led_ctl.adjust_time_cnt = 0;
                return;
            }
#endif

            if (led_ctl.adjust_time_cnt >= PWM_DUTY_SLOW_ADJUST_UNIT)
            {
                led_ctl.adjust_time_cnt = 0;
                led_ctl.cur_pwm_duty_val++;
                if (led_ctl.status == LED_STATUS_YELLOW ||
                    led_ctl.status == LED_STATUS_WHITE_YELLOW)
                {
                    pwm_set_channel_0_duty(led_ctl.cur_pwm_duty_val);
                }

                if (led_ctl.status == LED_STATUS_WHITE ||
                    led_ctl.status == LED_STATUS_WHITE_YELLOW)
                {
                    pwm_set_channel_1_duty(led_ctl.cur_pwm_duty_val);
                }
            }
        }
        else if (led_ctl.cur_pwm_duty_val > led_ctl.dest_pwm_duty_val)
        {
            // 如果当前占空比 大于 目标占空（当前灯光亮度小于目标亮度值）
            // 要按照 充电期间 的速度进行调节
            if (led_ctl.adjust_time_cnt >= PWM_DUTY_SLOW_ADJUST_UNIT_DURING_CHARGING)
            {
                led_ctl.adjust_time_cnt = 0;
                led_ctl.cur_pwm_duty_val--;
                if (led_ctl.status == LED_STATUS_YELLOW ||
                    led_ctl.status == LED_STATUS_WHITE_YELLOW)
                {
                    pwm_set_channel_0_duty(led_ctl.cur_pwm_duty_val);
                }

                if (led_ctl.status == LED_STATUS_WHITE ||
                    led_ctl.status == LED_STATUS_WHITE_YELLOW)
                {
                    pwm_set_channel_1_duty(led_ctl.cur_pwm_duty_val);
                }
            }
        }
    }

#if USER_DEBUG_ENABLE
#if 0
    // USER_TO_DO 每隔一段时间，再打印一次占空比
    {
        static u8 cnt = 0;
        cnt++;
        if (cnt >= 100)
        {
            cnt = 0;

            // printf("PWM_DUTY_VAL_PERCENT_X(100 - 70) == %u\n", PWM_DUTY_VAL_PERCENT_X(100 - 70));
            // printf("led_ctl.dest_pwm_duty_val == %u\n", led_ctl.dest_pwm_duty_val);
            // printf("cur_pwm_duty_val = %u\n", led_ctl.cur_pwm_duty_val);
        }
    }
#endif
#endif
}

/**
 * @brief 充电或放电时，电池电量指示灯的动画
 *
 * 外部参数： 全局变量 bat_percent ，电池电量百分比
 *
 * 目前 1ms 调用一次
 *
 */
// void led_bat_instruction_timer_callback(void)
// {
// }
