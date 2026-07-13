#include "charge_det.h"
#include "user_include.h"
#include "bat_scan.h"

// volatile u8 charge_det_sta; // 检测充电的状态
volatile u8 is_in_charging_by_solar_panel = 0; // 是否正在通过太阳能充电
volatile u8 is_in_charging_by_charger = 0;     // 是否正在通过充电器充电

volatile u8 is_in_charging = 0;      // 是否正在充电
volatile u8 is_charging_ic_stop = 0; // 充电ic是否停止充电

volatile bit flag_is_time_to_det_charge = 0; // 是否到了检测充电状态的时机

// volatile u8 is_not_charging_by_charger_cnt = 0;

volatile u8 is_charging_by_solar_panel_cnt = 0;
volatile u8 is_not_charging_by_solar_panel_cnt = 0;

volatile u8 is_detect_discharge_signal = 0; // 是否检测到了连续的放电信号
volatile u8 is_in_discharging = 0;          // 是否正在放电

void charge_det_init(void)
{
    // 检测充电ic的两个引脚
    // P22 CH2 , P21 CH1
    P2_MD0 &= ~GPIO_P22_MODE_SEL(0x03); // 输入模式
    P2_PU |= GPIO_P22_PULL_UP(0x01);
    // P2_PD |= GPIO_P22_PULL_PD(0x01);

    P2_MD0 &= ~GPIO_P21_MODE_SEL(0x03); // 输入模式
    P2_PU |= GPIO_P21_PULL_UP(0x01);
    // P2_PD |= GPIO_P21_PULL_PD(0x01);
}

// 新增：专门用于检测1KHz信号的函数
// 每100微秒调用一次，用来检测 P21 引脚上的1KHz信号
void detect_1khz_signal_100us(void)
{
    // 新增变量用于检测1KHz信号
    static volatile u16 signal_high_duration = 0; // 高电平持续时间计数
    static volatile u16 signal_low_duration = 0;  // 低电平持续时间计数

    static volatile u8 detection_state = 0; // 检测状态：0-初始状态，1-检测到高电平，2-检测到低电平

    static volatile u16 toggle_count_in_period = 0;     // 一个预期的1KHz周期内的翻转次数
    static volatile u16 measurement_period_counter = 0; // 测量周期计数器
    static volatile u8 is_detected_signal = 0;          // 标志位，是否检测到1KHz信号

    // static volatile u8 last_pin_state = 0; // 上一次检测到的引脚状态
    u8 current_pin_state = CHARGE_IC_CH1; // 当前引脚状态
    // 如果连续多次检测到1KHz信号模式，则设置充电标志
    static volatile u16 consecutive_detection_count = 0; // 计数器，是否连续检测到多次充电信号（用于检测是否有充电）

    // 未检测到充电信号的计数器
    static volatile u16 undetected_charging_count = 0;

    // 增加测量周期计数
    measurement_period_counter++;

    // 检测到低电平，并且之前没有检测到充电信号，可能不在充电
    if (current_pin_state == 0 && consecutive_detection_count == 0)
    {
        undetected_charging_count++;
        if (undetected_charging_count >= (u16)2000 * 10) // 时间单位：100us
        {
            // 目前至少持续1s以上，才能确认充电ic没有在充电
            undetected_charging_count = 0;
            is_in_charging_by_charger = 0;
            is_charging_ic_stop = 0;
            consecutive_detection_count = 0;
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

        if (toggle_count_in_period >= 16) // 1KHz信号，10ms内有20次翻转，这里检测到有16次就认为满足条件
        {                                 // 在这段时间内，至少检测到   多少次翻转
            is_detected_signal = 1;
        }
        else if (toggle_count_in_period <= 10)
        {
            // 实际测试发现，哪怕是1KHz连续的信号，也会有大概率(50%左右)进入这里
            is_detected_signal = 0;
        }
        toggle_count_in_period = 0;
    }

    if (is_detected_signal)
    {
        consecutive_detection_count++;
        if (consecutive_detection_count >= 100) // 100 * 100us，连续 10ms 检测到1Khz信号
        {
            is_in_charging_by_charger = 1;
        }

        // 5000 *10 * 100 us 连续 5s 检测到1Khz信号，说明充满电(实际测试可能只有3s)
        if (consecutive_detection_count >= (u32)5000 * 10)
        {
            is_charging_ic_stop = 1;
            // consecutive_detection_count = 500; // 限制最大值，防止溢出
            consecutive_detection_count = (u32)5000 * 10; // 限制最大值，防止溢出
        }
    }
    else
    {
        if (consecutive_detection_count > 0)
        {
            consecutive_detection_count--;
        }
    }

    // last_pin_state = current_pin_state;
}

// 检测充电ic的1Khz信号
void detect_discharge_1khz_signal_100us(void)
{
    // 新增变量用于检测1KHz信号
    static volatile u16 signal_high_duration = 0; // 高电平持续时间计数
    static volatile u16 signal_low_duration = 0;  // 低电平持续时间计数

    /*
        检测状态：
        0 - 初始状态，
        1 - 检测到高电平，
        2 - 检测到低电平
    */
    static volatile u8 detection_state = 0;

    static volatile u16 toggle_count_in_period = 0;     // 一个预期的1KHz周期内的翻转次数
    static volatile u16 measurement_period_counter = 0; // 测量周期计数器

    u8 current_pin_state = CHARGE_IC_CH2; // 当前引脚状态

    // =================================================================
    // 在最后判断是否真的有放电信号
    static volatile u8 is_detected_signal = 0; // 标志位，是否检测到1KHz信号

    static volatile u8 detect_discharge_signal_cnt = 0;   // 检测到放电信号的计数器
    static volatile u8 undetect_discharge_signal_cnt = 0; // 未检测到放电信号的计数器

    // =================================================================

    // 增加测量周期计数
    measurement_period_counter++;

    /*
        检测低电平和高电平的持续时间，并且累计电平翻转次数
    */
    switch (detection_state)
    {
    case 0: // 初始状态，等待进入高电平
        if (current_pin_state == 1)
        {
            /*
                检测到高电平
                进入高电平检测状态
            */
            detection_state = 1;
            signal_high_duration = 1; // 高电平时间计数加一
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

        if (toggle_count_in_period >= 16) // 1KHz信号，10ms内有20次翻转，这里检测到有16次就认为满足条件
        {                                 // 在这段时间内，至少检测到   多少次翻转
            is_detected_signal = 1;
        }
        else if (toggle_count_in_period <= 8)
        {
            is_detected_signal = 0;
        }

        // 处理完成后，清空计数，给下一个检测周期继续检测
        toggle_count_in_period = 0;

        if (is_detected_signal)
        {
            undetect_discharge_signal_cnt = 0;
            detect_discharge_signal_cnt++;
            if (detect_discharge_signal_cnt >= 100) // 100 * 10ms
            {
                detect_discharge_signal_cnt = 0;
                // is_in_discharging = 1;
                is_detect_discharge_signal = 1;
            }
        }
        else
        {
            detect_discharge_signal_cnt = 0;
            undetect_discharge_signal_cnt++;
            if (undetect_discharge_signal_cnt >= 100) // 100 * 10ms
            {
                undetect_discharge_signal_cnt = 0;
                // is_in_discharging = 0;
                is_detect_discharge_signal = 0;
            }
        }
    }
}

void charge_det_time_add(void)
{
    flag_is_time_to_det_charge = 1;
}

// extern volatile u16 adc_type_c_det_val;
//
void charge_det(void)
{
    static u16 adc_val_of_solar_panel = 0;
    static u16 adc_val_of_type_c = 0;
    u16 voltage_mv = 0;

    static volatile u8 is_discharge_det_time_cnt = 0; // 检测到正在放电的计数器
    // static volatile u8 isnot_discharge_det_time_cnt = 0; // 检测到不是正在放电的计数器

    if (adc_get_update_flag(ADC_CHANNEL_SEL_SOLAR_DET))
    {
        adc_clear_update_flag(ADC_CHANNEL_SEL_SOLAR_DET);
        adc_val_of_solar_panel = adc_get_val(ADC_CHANNEL_SEL_SOLAR_DET);
    }

    if (adc_get_update_flag(ADC_CHANNEL_SEL_TYPE_C))
    {
        adc_clear_update_flag(ADC_CHANNEL_SEL_TYPE_C);
        adc_val_of_type_c = adc_get_val(ADC_CHANNEL_SEL_TYPE_C);
    }

    if (flag_is_time_to_det_charge)
    {
        flag_is_time_to_det_charge = 0;
    }
    else
    {
        // 检测时间没有到，直接返回
        return;
    }

    /*
        将采集到的太阳能一侧的ad值转换为实际的电压值
        太阳能一侧的电压 == ad值 / 4096 * 参考电压 * 分压系数
        从太阳能到电池的电压 == 太阳能一侧的电压 - 0.3V

        如果检测到太阳能一侧的电压比电池电压还要大一点，认为在通过太阳能充电
        暂定比电池电压还要大0.30V
    */
    voltage_mv = (u32)adc_val_of_solar_panel * 2 * 2400 / 4096;
    // if (voltage_mv >= 4500) // 大于 4.5V，认为是正在给电池充电
    if (voltage_mv >= (avg_voltage_mv + 300)) // 比电池电压还要大 0.30 V
    {
        is_not_charging_by_solar_panel_cnt = 0;
        is_charging_by_solar_panel_cnt++;
        if (is_charging_by_solar_panel_cnt >= 200) // 目前单位：ms
        {
            is_charging_by_solar_panel_cnt = 0;
            // 累计一段时间后，才认为有充电
            is_in_charging_by_solar_panel = 1;

#if USER_DEBUG_ENABLE
            // printf("is charging by solar panel\n");
#endif
        }
    }
    else
    {
        is_charging_by_solar_panel_cnt = 0;
        is_not_charging_by_solar_panel_cnt++;
        if (is_not_charging_by_solar_panel_cnt >= 200) // 目前单位：ms
        {
            is_not_charging_by_solar_panel_cnt = 0;
            // 累计一段时间后，才认为没有充电
            is_in_charging_by_solar_panel = 0;

#if USER_DEBUG_ENABLE
            // printf("is not charging by solar panel\n");
#endif
        }
    }

/**
 *    目前测试，使用2V参考电压，
 *    检测脚外部10K上拉、10K下拉，或者是100K上拉，100K下拉
 *    只插入type-c充电，检测到的ad值会在4095
 *    只通过太阳能一侧充电，检测到的ad值会在100附近（可以忽略不计）
 *
 *    这里只要小于2000，就认为没有通过type-c充电
 *
 *    REVIEW 如果在旧的板子上测试，
 *    旧的板子引脚连接到的是VDD，而不是type-c分压后的电压
 *    测试完成后，需要恢复这部分程序
 */
#if USER_DEBUG_ENABLE
    // printf("adc_val_of_type_c == %u\n", adc_val_of_type_c);
#endif
    if (adc_val_of_type_c <= 2000 ||
        voltage_mv < (avg_voltage_mv + 200))
    {
        // 充电口电压已经很低，或者太阳能侧输入已经不再有效时，
        // 说明此时不是实际放电，直接清掉放电状态，避免误判。
        is_discharge_det_time_cnt = 0;
        // isnot_discharge_det_time_cnt = 0;
        is_in_discharging = 0;
        is_detect_discharge_signal = 0;
    }
    else if (adc_val_of_type_c >= 4000)
    {
        // 充电口的电压接近 5V，认为在通过 type-C 进行充电或者放电
        if (is_detect_discharge_signal)
        {
            // isnot_discharge_det_time_cnt = 0;
            is_discharge_det_time_cnt++;
            if (is_discharge_det_time_cnt >= 200)
            {
                is_discharge_det_time_cnt = 0;
                // 累计一段时间后，才认为正在放电
                is_in_discharging = 1;
            }
        }
    }
    else
    {
        // 处于 2V~4V 之间的过渡区间时，先不作为有效放电判定，
        // 避免在输入电压下降时把“弱输入/误触发”误认为真正放电。
        is_discharge_det_time_cnt = 0;
        // isnot_discharge_det_time_cnt = 0;
        is_in_discharging = 0;
    }

    // printf("adc_val_of_solar_panel == %u\n", adc_val_of_solar_panel);

    if (is_in_charging_by_charger || is_in_charging_by_solar_panel)
    {
        is_in_charging = 1;

        // 有充电之后，清空低电量相关标志位
        is_sent_low_bat_alert = 0; 
        is_in_low_bat_alert = 0;
    }
    else if (is_in_charging_by_charger == 0 && is_in_charging_by_solar_panel == 0)
    {
        is_in_charging = 0;
    }

    // 充电时关机：
    // 检测到正在充电时，关闭蓝牙，关闭灯光
    // if (is_in_charging)
    // {
    //     if (led_ctl.status != LED_STATUS_OFF)
    //     {
    //         led_status_set(LED_STATUS_OFF); // 关闭灯光
    //     }

    //     if (ble_ic.is_working)
    //     {
    //         ble_ic_disable_pre();
    //         delay_ms(30); // 等待蓝牙关闭
    //     }
    // }
}