#include "charge_det.h"
#include "user_include.h"

volatile u8 charge_det_sta; // 检测充电的状态
volatile u8 is_in_charging = 0;

// 新增变量用于检测1KHz信号
static u16 signal_high_duration = 0;       // 高电平持续时间计数
static u16 signal_low_duration = 0;        // 低电平持续时间计数
static u8 detection_state = 0;             // 检测状态：0-初始状态，1-检测到高电平，2-检测到低电平
static u16 toggle_count_in_period = 0;     // 一个预期的1KHz周期内的翻转次数
static u16 measurement_period_counter = 0; // 测量周期计数器
static u8 detected_signal_pattern = 0;     // 标记是否检测到1KHz信号模式

void charge_det_init(void)
{
    // 检测充电ic的两个引脚
    // P22 CH2 , P21 CH1
    P2_MD0 &= ~GPIO_P22_MODE_SEL(0x03); // 输入模式
    P2_PU |= GPIO_P22_PULL_UP(0x01);

    P2_MD0 &= ~GPIO_P21_MODE_SEL(0x03); // 输入模式
    P2_PU |= GPIO_P21_PULL_UP(0x01);
}

// 5000 us / 100us
#define CHARGE_IC_SCAN_PERIOD ((u16)5000 / 100) // 扫描充电ic引脚的周期，单位：100us
void charge_ic_sacn_100us_isr(void)
{
    static u16 cur_scan_cnt = 0; // 记录当前扫描的周期计数
    static u16 toggle_cnt = 0;
    static u8 last_pin_level = 0;
    u8 cur_pin_level = P21;

    if (P22 != 0)
    {
        // 清空相关计数，重新开始扫描
        toggle_cnt = 0;
        cur_scan_cnt = 0;
        return;
    }

    cur_scan_cnt++;
    /*
        P22 == 0 并且检测到 P21 有1KHz的信号，说明正在充电
    */
    if (cur_pin_level != last_pin_level)
    {
        toggle_cnt++;
        last_pin_level = cur_pin_level;
    }

    if (cur_scan_cnt >= CHARGE_IC_SCAN_PERIOD)
    {
        cur_scan_cnt = 0;

        /*
            1KHz 的信号，在一个扫描周期内
        */
        if (toggle_cnt >= 8)
        {
            toggle_cnt = 0;
            is_in_charging = 1;
        }
    }
}

// 新增：专门用于检测1KHz信号的函数
// 每100微秒调用一次，用来检测P21引脚上的1KHz信号
void detect_1khz_signal_100us(void)
{
    static u8 last_pin_state = 0;
    u8 current_pin_state = P21;
    u8 pin_changed = (current_pin_state != last_pin_state);
    // 如果连续多次检测到1KHz信号模式，则设置充电标志
    static u8 consecutive_detection_count = 0;

    // 检查 是否为低电平，如果 不是低电平，则不进行检测
    if (P22 != 0)
    {
        // 重置所有状态
        detection_state = 0;
        signal_high_duration = 0;
        signal_low_duration = 0;
        toggle_count_in_period = 0;
        measurement_period_counter = 0;
        detected_signal_pattern = 0;
        is_in_charging = 0;
        last_pin_state = current_pin_state;
        return;
    }

    // 增加测量周期计数
    measurement_period_counter++;

    switch (detection_state)
    {
    case 0: // 初始状态，等待进入高电平
        if (current_pin_state == 1)
        {                        // 检测到高电平
            detection_state = 1; // 进入高电平检测状态
            signal_high_duration = 1;
            signal_low_duration = 0;
        }
        else
        {
            signal_high_duration = 0;
            signal_low_duration = 1;
        }
        break;

    case 1: // 检测高电平持续时间
        if (current_pin_state == 1)
        { // 仍在高电平
            signal_high_duration++;
            signal_low_duration = 0;
        }
        else
        { // 检测到下降沿，进入低电平检测
            detection_state = 2;
            signal_low_duration = 1;

            // 检查高电平持续时间是否接近1KHz的半周期（即0.5ms = 5000*0.5/100 = 25个100us周期）
            // 允许一定误差范围，比如20到30个100us周期之间
            if (signal_high_duration >= 20 && signal_high_duration <= 30)
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

            // 检查低电平持续时间是否接近1KHz的半周期
            if (signal_low_duration >= 20 && signal_low_duration <= 30)
            {
                toggle_count_in_period++; // 半周期匹配1KHz信号
            }
            signal_low_duration = 0;
        }
        break;
    }

    // 如果在一个大约1ms的时间窗口内检测到足够多的1KHz半周期信号
    // 1ms大约是10个100us周期，所以如果检测到了8次或以上的半周期信号，就认为是1KHz信号
    if (measurement_period_counter >= 10)
    { // 大约1ms
        measurement_period_counter = 0;

        // 如果在1ms内检测到了足够的1KHz信号半周期，则确认是1KHz信号
        if (toggle_count_in_period >= 8)
        { // 至少检测到4个完整周期的1KHz信号
            detected_signal_pattern = 1;
        }
        else
        {
            detected_signal_pattern = 0;
        }
        toggle_count_in_period = 0;
    }

    if (detected_signal_pattern)
    {
        consecutive_detection_count++;
        if (consecutive_detection_count >= 10)
        { // 连续10ms检测到1KHz信号
            is_in_charging = 1;
            consecutive_detection_count = 10; // 限制最大值，防止溢出
        }
    }
    else
    {
        consecutive_detection_count = 0;
        is_in_charging = 0;
    }

    last_pin_state = current_pin_state;
}

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
    // if (voltage_mv >= 4500) // 大于 4.5V，认为是正在给电池充电
    // {
    //     is_in_charging = 1;
    // }
    // else
    // {
    //     is_in_charging = 0;
    // }
}