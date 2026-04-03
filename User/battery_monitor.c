#include "battery_monitor.h"
#include "user_include.h"

// volatile u8 is_send_low_battery_enable = 0;

// 是否发送了低电量报警（ USER_TO_DO 在第一次上电、低功耗唤醒之后、有充电之后，需要清零）
volatile u8 is_sent_low_bat_alert = 0;
volatile u8 is_turn_off_by_low_bat = 0; // 是否低电量关机（ USER_TO_DO 从低功耗唤醒，有充电之后，都清除一下该标志位）

// ====================================================================
static volatile u8 is_bat_vol_buff_add_enable = 0;     // 是否允许往电池电压数组中放入数据
static volatile u8 is_bat_vol_buff_get_avg_enable = 0; // 是否允许往电池电压数组中获取平均值

// 控制一段时间内采集电池电压峰值（电压最大值）
static volatile bat_vol_update_sta_t bat_vol_update_sta = BAT_VOL_UPDATE_STA_IDLE;
static volatile u16 bat_vol_update_cnt = 0;

// 记录一段时间内采集到的电压值：
static volatile u16 bat_vol_history_buff[VOLTAGE_HISTORY_SIZE] = {0};
static volatile u8 bat_vol_history_buff_idx = 0;
// ====================================================================

// 只在采集使用：
static volatile u16 voltage_mv = 0;     //
static volatile u16 max_voltage_mv = 0; // 存放一段时间内采集到的最大电压值（只在采集使用，不能作为最终的判断使用）

// 最后得到的、稳定的电池电压
static volatile u16 avg_voltage_mv = 0;

static volatile u8 percent = 0;      // 电池电量百分比
static volatile u8 last_percent = 0; // 记录上一次采集到的电量百分比，用来控制电量百分比更新，充电时百分比不能下降，放电时百分比不能上升

// 最终得到的、稳定的电池电量百分比
volatile u8 bat_percent = 0;

// 电池电量低时，每隔 xx 时间，给蓝牙ic发送一次提示，单位：ms
#define SEND_LOW_BAT_TIME_PERIOD ((u16)30 * 1000)
static volatile u8 is_send_low_bat_repeatedly_enable = 0; // 是否重复发送低电量报警

// 控制发送低电量的周期
void send_low_bat_timer_callback(void)
{
    static volatile u16 cnt = 0;
    if (is_sent_low_bat_alert)
    {
        cnt++;
        if (cnt >= SEND_LOW_BAT_TIME_PERIOD)
        {
            cnt = 0;
            is_send_low_bat_repeatedly_enable = 1;
        }
    }
    else
    {
        cnt = 0;
    }
}

// 由定时器调用，控制一段时间内，往数组中放入数据的时间
void bat_vol_buff_add_timer_callback(void)
{
    static u32 cnt = 0;
    cnt++;
    if (cnt >= BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER)
    {
        cnt = 0;
        is_bat_vol_buff_add_enable = 1;
    }
}

// 由定时器调用，控制一段时间内，从数组中获取平均值的时间
void bat_vol_buff_get_avg_timer_callback(void)
{
    static u32 cnt = 0;
    cnt++;
    if (cnt >= BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER_EXTRACT)
    {
        cnt = 0;
        is_bat_vol_buff_get_avg_enable = 1;
    }
}

// 由定时器调用，控制一段时间，获取电池电压的最大值（测试发现在充电和放电时，一段时间内电池电压的最大值是最接近电池电压实际值的）
void bat_vol_update_timer_callback(void)
{
    if (bat_vol_update_sta == BAT_VOL_UPDATE_STA_IDLE)
    {
        bat_vol_update_cnt = 0;
        bat_vol_update_sta = BAT_VOL_UPDATE_STA_CAPTURING;
    }
    else if (bat_vol_update_sta == BAT_VOL_UPDATE_STA_COMPLETED)
    {
        // 已经采集完成，直接返回
        return;
    }

    bat_vol_update_cnt++;
    if (bat_vol_update_cnt >= BATTERY_VOLTAGE_UPDATE_PERIOD)
    {
        /*
            这里可以不用清零，只有 bat_vol_update_sta
            刚进入 BAT_VOL_UPDATE_STA_IDLE 时清零
        */
        // bat_vol_update_cnt = 0;
        bat_vol_update_sta = BAT_VOL_UPDATE_STA_COMPLETED;
    }
}

// 根据ad值，转换成对应的电池电压值
u16 get_battery_voltage_by_adc(u16 adc_val)
{
    u16 voltage_mv = ADC_TO_BATTERY_VOLTAGE_MV(adc_val);
    // printf("voltage_mv == %u\n", voltage_mv); // 打印转换好的电压值（发现实际打印的电压比万用表量出的电压大了0.13V）
    if (voltage_mv > 130)
    {
        voltage_mv -= 130; // 这里做电压补偿（减去0.13V）
    }
    // printf("voltage_mv == %u\n", voltage_mv); //

    if (is_in_charging)
    {
        //  // 充电时，减去 0.1 V，作为补偿。如果电压接近 4.2V 时，不用补偿
        if ((voltage_mv <= 4000) && voltage_mv > 100)
        {
            voltage_mv -= 100;
        }
    }

    return voltage_mv;
}

// void send_low_battery_timer_callback(void)
// {
//     static u32 cnt = 0;
//     cnt++;
//     if (cnt >= (u32)30 * 1000)
//     {
//         is_send_low_battery_enable = 1;
//         cnt = 0;
//     }
// }

// 根据电压值计算初始电量百分比 (线性估算)
u8 get_battery_percentage_by_voltage(u16 voltage_mv)
{
    u32 percentage;
    if (voltage_mv >= BATTERY_FULL_VOLTAGE)
    {
        return 100;
    }
    else if (voltage_mv <= BATTERY_EMPTY_VOLTAGE)
    {
        return 0;
    }

    // 线性插值计算百分比
    percentage = ((u32)(voltage_mv - BATTERY_EMPTY_VOLTAGE) * 100) /
                 (BATTERY_FULL_VOLTAGE - BATTERY_EMPTY_VOLTAGE);

    return (u8)percentage > 100 ? 100 : (u8)percentage;
}

void bat_vol_history_buff_init(u16 voltage_mv)
{
    u8 i;
    for (i = 0; i < VOLTAGE_HISTORY_SIZE; i++)
    {
        bat_vol_history_buff[i] = voltage_mv;
    }
    bat_vol_history_buff_idx = 0;
}

void bat_vol_history_buff_add(u16 voltage_mv)
{
    bat_vol_history_buff[bat_vol_history_buff_idx] = voltage_mv;
    bat_vol_history_buff_idx = (bat_vol_history_buff_idx + 1) % VOLTAGE_HISTORY_SIZE;
    // bat_vol_history_buff_idx++;
    // if (bat_vol_history_buff_idx >= VOLTAGE_HISTORY_SIZE)
    // {
    //     bat_vol_history_buff_idx = 0;
    // }
}

u16 bat_vol_history_buff_get_avg(void)
{
    u8 i;
    u32 ret = 0;
    for (i = 0; i < VOLTAGE_HISTORY_SIZE; i++)
    {
        ret += bat_vol_history_buff[i];
    }
    return (u16)(ret / VOLTAGE_HISTORY_SIZE);
}

// 在低功耗唤醒后调用，由唤醒后第一次采集到的ad值来初始化相关数据
// void battery_monitor_init_by_adc_val(u16 adc_val)
// 强制刷新
void battery_monitor_refresh_by_adc_val(u16 adc_val)
{
    voltage_mv = get_battery_voltage_by_adc(adc_val);
    bat_vol_history_buff_init(voltage_mv);
    max_voltage_mv = voltage_mv;
    avg_voltage_mv = voltage_mv;
    percent = get_battery_percentage_by_voltage(voltage_mv);
    last_percent = percent;
    bat_percent = percent;
    bat_vol_update_sta = BAT_VOL_UPDATE_STA_COMPLETED;
}

#if USER_DEBUG_ENABLE
// 测试用，手动改变得到的电池电压
void user_test_init_by_voltage_mv(u16 test_voltage_mv)
{
    // printf("test_voltage_mv == %u\n", test_voltage_mv);
    voltage_mv = test_voltage_mv;
    bat_vol_history_buff_init(voltage_mv);
    max_voltage_mv = voltage_mv;
    avg_voltage_mv = voltage_mv;
    percent = get_battery_percentage_by_voltage(voltage_mv);
    last_percent = percent;
    bat_percent = percent;
    bat_vol_update_sta = BAT_VOL_UPDATE_STA_COMPLETED;

    // printf("voltage_mv == %u\n", voltage_mv);
    // printf("max_voltage_mv == %u\n", max_voltage_mv);
    // printf("avg_voltage_mv == %u\n", voltage_mv);
    // printf("percent == %u\n", (u16)percent);
    // printf("last_percent == %u\n", (u16)last_percent);
    // printf("bat_percent == %u\n", (u16)bat_percent);
}
#endif

// 修改后的电池监控处理函数
void battery_monitor_handle(void)
{
    volatile u16 adc_val = 0;
    volatile u16 cur_voltage_mv = 0; // 存放当前采集到的电压值

#if 1
    // 获取AD值（ad值有更新才获取）
    if (adc_get_update_flag(ADC_CHANNEL_SEL_BAT_DET))
    {
        adc_clear_update_flag(ADC_CHANNEL_SEL_BAT_DET);
        adc_val = adc_get_val(ADC_CHANNEL_SEL_BAT_DET);
        // printf("bat adc val == %u\n", adc_val);
        cur_voltage_mv = get_battery_voltage_by_adc(adc_val);

        if (0 == voltage_mv)
        {
            // 如果还没有采集过电池电压，直接将第一次采集到的ad值转换成电池电压
            voltage_mv = cur_voltage_mv;

            bat_vol_history_buff_init(voltage_mv);
            avg_voltage_mv = bat_vol_history_buff_get_avg();
            percent = get_battery_percentage_by_voltage(avg_voltage_mv);
            last_percent = percent;
            bat_percent = percent; // 初始化全局变量，电池电量百分比

#if USER_DEBUG_ENABLE
            printf("bat monitor init\n");
            printf("avg_voltage_mv == %u\n", avg_voltage_mv);
            printf("percent == %u\n", (u16)percent);
#endif
        }

        // 采集一段时间的电压值，取其中最大的值作为电池电压
        /*
            USER_TO_DO
            测试时，如果需要改变电池电压和电量百分比，需要屏蔽下面这段程序。
            否则会更新 voltage_mv 和 max_voltage_mv，可能会被赋值为0，
            导致又进入了 电池电量相关变量的初始化
        */
#if 1
        if (bat_vol_update_sta == BAT_VOL_UPDATE_STA_COMPLETED)
        {
            voltage_mv = max_voltage_mv;
            max_voltage_mv = 0;
            bat_vol_update_sta = BAT_VOL_UPDATE_STA_IDLE;

            // 输出计算结果 (调试用)
            // printf("Voltage: %umV\n", voltage_mv);
        }
        else if (bat_vol_update_sta == BAT_VOL_UPDATE_STA_CAPTURING)
        {
            if (max_voltage_mv < cur_voltage_mv)
            {
                max_voltage_mv = cur_voltage_mv;
            }
        }
#endif
    }
#endif

    // 测试时使用：
    // if (cur_voltage_mv != 0)
    // {
    //     printf("cur_voltage_mv == %u\n", cur_voltage_mv);
    // }

    if (voltage_mv == 0)
    {
        return; // 尚未获取到电压值，不执行以下操作
    }

    // 每隔一段时间，将采集到的电压值放入数组
    if (is_bat_vol_buff_add_enable)
    {
        bat_vol_history_buff_add(voltage_mv);
        is_bat_vol_buff_add_enable = 0;
    }

    // 每隔一段时间，将数组中的数据进行平均
    if (is_bat_vol_buff_get_avg_enable)
    {
        avg_voltage_mv = bat_vol_history_buff_get_avg();
        // printf("avg_voltage_mv == %u\n", avg_voltage_mv);

        percent = get_battery_percentage_by_voltage(avg_voltage_mv);
        // printf("percent == %u\n", (u16)percent);

        is_bat_vol_buff_get_avg_enable = 0;
    }

    // 充电中，percent大于等于last_percent，不让percent小于last_percent
    if (is_in_charging)
    {
        if (percent < last_percent)
        {
            percent = last_percent;
        }
        else
        {
            last_percent = percent;
        }
    }
    // 放电中，percent小于等于last_percent，不让percent大于last_percent
    else
    {
        if (percent > last_percent)
        {
            percent = last_percent;
        }
        else
        {
            last_percent = percent;
        }
    }

    bat_percent = percent; // 得到稳定之后的电池电量百分比

#if USER_DEBUG_ENABLE
#if 1
    // USER_TO_DO 只在测试时使用：
    // 每隔一段时间打印一次滤波后得到的电压和电量百分比
    if (flag_debug)
    {
        flag_debug = 0;
        printf("avg_voltage_mv == %u\n", avg_voltage_mv);
        printf("bat_percent == %u\n", (u16)bat_percent); //
    }
#endif
#endif

    // 控制电池电量指示灯
    if (is_in_charging &&
        (led_bat_level_sta != LED_BAT_LEVEL_STA_CHARGE_BEGIN &&
         led_bat_level_sta != LED_BAT_LEVEL_STA_CHARGE_BEGIN_ANIM &&
         led_bat_level_sta != LED_BAT_LEVEL_STA_CHARGING &&
         led_bat_level_sta != LED_BAT_LEVEL_STA_CHARGE_END))
    {
        led_bat_level_sta = LED_BAT_LEVEL_STA_CHARGE_BEGIN;
    }
    // 不在充电、蓝牙ic不工作、灯光关闭时，关闭电量指示灯
    else if (is_in_charging == 0 &&
             ble_ic.is_working == 0 &&
             led_ctl.status == LED_STATUS_OFF)
    {
        led_bat_level_sta = LED_BAT_LEVEL_STA_IDLE;

        // 关机后（不在工作），需要在这里清空对应标志位
    }
    // 不在充电，但是蓝牙ic或者灯光打开，根据电池电量来点亮对应指示灯
    else if (is_in_charging == 0 &&
             (ble_ic.is_working || led_ctl.status != LED_STATUS_OFF))
    {
        led_bat_level_sta = LED_BAT_LEVEL_STA_DISCHARGE;

        // 到了关机对应的电压
        if (avg_voltage_mv <= BATTERY_EMPTY_VOLTAGE)
        {
#if USER_DEBUG_ENABLE
            printf("detect bat power empty\n");
#endif
            if (0 == is_turn_off_by_low_bat)
            {
                // 关闭灯光
                if (led_ctl.status != LED_STATUS_OFF)
                {
                    led_status_set(LED_STATUS_OFF);
                }

                // 关闭蓝牙
                if (ble_ic.is_working)
                {
                    uart_data_send_cmd(UART_SEND_CMD_LOW_POWER_SHUTDOWN);
                    delay_ms(5000); // 这里要等蓝牙播报完成
                    ble_ic_disable_pre();
                    delay_ms(100);
                }

                is_turn_off_by_low_bat = 1;
            }
        }
        /*
            低电量，并且没有发送过低电量报警，要打开蓝牙，发送低电量报警
        */
        else if (is_sent_low_bat_alert == 0 &&
                 avg_voltage_mv <= BATTERY_LOW_WARNING_VOLTAGE)
        {
            // u8 i; // 循环计数值，控制连续发送低电量报警的次数
#if USER_DEBUG_ENABLE
            printf("detect low power\n");
#endif
            if (ble_ic.is_working == 0)
            {
                ble_ic_enable();
            }

            // for (i = 0; i < 1; i++)
            {
                uart_data_send_cmd(UART_SEND_CMD_LOW_POWER_WARNING);
            }

            is_sent_low_bat_alert = 1;
        }

        // 灯开着，或者蓝牙正在工作，并且到了电池低电量状态，每隔一段时间发送一次低电量报警：
        if (is_send_low_bat_repeatedly_enable)
        {
            uart_data_send_cmd(UART_SEND_CMD_LOW_POWER_WARNING);
            is_send_low_bat_repeatedly_enable = 0;

#if USER_DEBUG_ENABLE
            printf("UART_SEND_CMD_LOW_POWER_WARNING\n");
#endif
        }
    }

    // 测试时使用，充电动画：
    // if (is_in_charging_by_charger)
    // {
    //     LED_100_PERCENT_ON();
    //     LED_75_PERCENT_ON();
    //     LED_50_PERCENT_ON();
    //     LED_25_PERCENT_ON();
    // }
    // else
    // {
    //     LED_100_PERCENT_OFF();
    //     LED_75_PERCENT_OFF();
    //     LED_50_PERCENT_OFF();
    //     LED_25_PERCENT_OFF();
    // }
}