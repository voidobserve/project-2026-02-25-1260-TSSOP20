#include "charge_det.h"
#include "user_include.h"

// volatile u8 charge_det_sta; // 检测充电的状态
static volatile u8 is_in_charging_by_solar_panel = 0; // 是否正在通过太阳能充电
static volatile u8 is_in_charging_by_charger = 0;     // 是否正在通过充电器充电

volatile u8 is_in_charging = 0;   // 是否正在充电
volatile u8 is_charging_ic_stop = 0; // 充电ic是否停止充电

void charge_det_init(void)
{
    // 检测充电ic的两个引脚
    // P22 CH2 , P21 CH1
    P2_MD0 &= ~GPIO_P22_MODE_SEL(0x03); // 输入模式
    P2_PU |= GPIO_P22_PULL_UP(0x01);

    P2_MD0 &= ~GPIO_P21_MODE_SEL(0x03); // 输入模式
    P2_PU |= GPIO_P21_PULL_UP(0x01);
}

// 新增：专门用于检测1KHz信号的函数
// 每100微秒调用一次，用来检测 P21 引脚上的1KHz信号
void detect_1khz_signal_100us(void)
{
    // 新增变量用于检测1KHz信号
    static u16 signal_high_duration = 0; // 高电平持续时间计数
    static u16 signal_low_duration = 0;  // 低电平持续时间计数

    static u8 detection_state = 0; // 检测状态：0-初始状态，1-检测到高电平，2-检测到低电平

    static u16 toggle_count_in_period = 0;     // 一个预期的1KHz周期内的翻转次数
    static u16 measurement_period_counter = 0; // 测量周期计数器
    static u8 is_detected_signal = 0;          // 标志位，是否检测到1KHz信号

    static u8 last_pin_state = 0;
    u8 current_pin_state = CHARGE_IC_CH1;
    u8 pin_changed = (current_pin_state != last_pin_state);
    // 如果连续多次检测到1KHz信号模式，则设置充电标志
    // static u8 consecutive_detection_count = 0; // 计数器，是否连续检测到多次充电信号（用于检测是否有充电）
    static u16 consecutive_detection_count = 0; // 计数器，是否连续检测到多次充电信号（用于检测是否有充电）
    // static u16 consecutive_detection_count_  // 计数器，是否连续检测到多次充电信号（用于检测是否充满电）

    // 未检测到充电信号的计数器
    static u16 undetected_charging_count = 0;

    // 增加测量周期计数
    measurement_period_counter++;

    // 检测到低电平，并且之前没有检测到充电信号，可能不在充电
    if (current_pin_state == 0 && consecutive_detection_count == 0)
    {
        undetected_charging_count++;
        if (undetected_charging_count >= (u16)2000 * 10) // 时间单位：100us
        {
            undetected_charging_count = 0;
            is_in_charging_by_charger = 0;
            is_charging_ic_stop = 0;
        }
    }
    else
    {
        undetected_charging_count = 0;
    }

    switch (detection_state)
    {
    case 0:                         // 初始状态，等待进入高电平
        if (current_pin_state == 1) // 检测到高电平
        {
            detection_state = 1; // 进入高电平检测状态
            signal_high_duration = 1;
            signal_low_duration = 0;
        }
        else // 检测到低电平
        {
            detection_state = 2; // 进入低电平检测状态
            signal_high_duration = 0;
            signal_low_duration = 1;
        }
        break;

    case 1: // 检测高电平持续时间
        if (current_pin_state == 1)
        {                                     // 仍在高电平
            if (signal_high_duration < 65535) // 避免计数溢出s
            {
                signal_high_duration++;
            }

            signal_low_duration = 0;
        }
        else
        { // 检测到下降沿，进入低电平检测
            detection_state = 2;
            signal_low_duration = 1;

            // 检查高电平持续时间是否接近1KHz的半个周期（即0.5ms == 500us = 5个100us周期）
            // 允许一定误差范围，比如3 ~ 5 个 100us 周期之间
            // user_debug_val_u16 = signal_high_duration; // 获取得到的计数值（测试时使用）
            if (signal_high_duration >= 3 && signal_high_duration <= 5)
            {
                toggle_count_in_period++; // 半周期匹配1KHz信号
            }
            signal_high_duration = 0;
        }
        break;

    case 2: // 检测低电平持续时间
        if (current_pin_state == 0)
        { // 仍在低电平
            signal_low_duration++;
            signal_high_duration = 0;
        }
        else
        { // 检测到上升沿，回到高电平检测
            detection_state = 1;
            signal_high_duration = 1;

            // user_debug_val_u16 = signal_low_duration; // 获取得到的计数值（测试时使用）

            // 检查低电平持续时间是否接近1KHz的半个周期
            if (signal_low_duration >= 3 && signal_low_duration <= 5)
            {
                toggle_count_in_period++; // 半周期匹配1KHz信号
            }

            signal_low_duration = 0;
        }
        break;
    }

    if (measurement_period_counter >= 100) // 100：100 * 100us == 10ms
    {
        measurement_period_counter = 0;

        // 如果 10 ms内检测到了足够的1KHz信号半周期，则确认是1KHz信号

        // user_debug_val_u16 = toggle_count_in_period; // 获取得到的计数值（测试时使用）

        if (toggle_count_in_period >= 16)
        { // 在这段时间内，至少检测到   多少次翻转
            is_detected_signal = 1;
        }
        else
        {
            is_detected_signal = 0;
        }
        toggle_count_in_period = 0;
    }

    if (is_detected_signal)
    {
#if 0
        consecutive_detection_count++;
        if (consecutive_detection_count >= 100) //  100 * 10ms 连续 1s检测到1Khz信号
        { // 连续  ms检测到1KHz信号
            is_in_charging_by_charger = 1;
            consecutive_detection_count = 100; // 限制最大值，防止溢出
        }
#endif

        consecutive_detection_count++;
        if (consecutive_detection_count >= 100) // 100 * 100us，连续 10ms 检测到1Khz信号
        {
            is_in_charging_by_charger = 1; 
        }

        // if (consecutive_detection_count >= 500) //  500 * 10ms 连续 5s检测到1Khz信号，说明充满电
        if (consecutive_detection_count >= (u32)5000 * 10) //  5000 *10 * 100 us 连续 5s检测到1Khz信号，说明充满电
        {
            is_charging_ic_stop = 1;
            // consecutive_detection_count = 500; // 限制最大值，防止溢出
            consecutive_detection_count = (u32)5000 * 10; // 限制最大值，防止溢出
        }
    }
    else
    {
        consecutive_detection_count = 0;
    }

    last_pin_state = current_pin_state;
}

// 
void charge_det(void)
{
    static u16 adc_val = 0;
    u16 voltage_mv = 0;

    if (adc_get_update_flag(ADC_CHANNEL_SEL_SOLAR_DET))
    {
        adc_clear_update_flag(ADC_CHANNEL_SEL_SOLAR_DET);
        adc_val = adc_get_val(ADC_CHANNEL_SEL_SOLAR_DET);
    }

    /*
        将采集到的太阳能一侧的ad值转换为实际的电压值
        太阳能一侧的电压 == ad值 / 4096 * 参考电压 * 分压系数
        从太阳能到电池的电压 == 太阳能一侧的电压 - 0.3V
    */
    voltage_mv = (u32)adc_val * 2 * 2400 / 4096;
    // printf("voltage_mv == %u\n", voltage_mv);
    if (voltage_mv >= 4500) // 大于 4.5V，认为是正在给电池充电
    {
        is_in_charging_by_solar_panel = 1;
    }
    else
    {
        is_in_charging_by_solar_panel = 0;
    }

    if (is_in_charging_by_charger || is_in_charging_by_solar_panel)
    {
        is_in_charging = 1;
    }
    else if (is_in_charging_by_charger == 0 && is_in_charging_by_solar_panel == 0)
    {
        is_in_charging = 0;
    }

    // 检测到正在充电时，关闭蓝牙，关闭灯光
    if (is_in_charging)
    {
        if (led_ctl.status != LED_STATUS_OFF)
        {
            led_status_set(LED_STATUS_OFF); // 关闭灯光
        }

        if (ble_ic.is_working)
        {
            ble_ic_disable_pre();
            delay_ms(30); // 等待蓝牙关闭
        }
    }
}