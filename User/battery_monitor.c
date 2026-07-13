#include "battery_monitor.h"
#include "user_include.h"

#include "led.h"
#include "bat_scan.h"

// 是否发送了低电量报警（ @NOTE 在第一次上电、低功耗唤醒之后、有充电之后，需要清零）
volatile u8 is_sent_low_bat_alert = 0;

// 电池电量低时，每隔 xx 时间，给蓝牙ic发送一次提示，单位：ms
#define SEND_LOW_BAT_TIME_PERIOD ((u32)120 * 1000)
// #define SEND_LOW_BAT_TIME_PERIOD ((u32)10 * 1000)         // TEST ONLY
/*
    是否重复发送低电量报警
    同时充当标志位，用来判断蓝牙有没有打开，如果刚从0变为1，打开蓝牙，进行低电量提示
*/
static volatile u8 is_send_low_bat_repeatedly_enable = 0;

// 是否处于低电量提示
// 有充电、进入低功耗，都清除该标志位：
volatile u8 is_in_low_bat_alert = 0; //
volatile u32 discharge_time_cnt = 0; // 放电时间计数，单位：s
volatile u32 charge_time_cnt = 0;    // 充电时间计数，单位：s

// 控制发送低电量的周期
void send_low_bat_timer_callback(void)
{
    static volatile u32 cnt = 0;
    /*
       刚打开蓝牙，发送低电量报警，
       需要隔一时间再发送，又不能隔太长时间，否则蓝牙会自动关机
       这里控制刚打开蓝牙之后，发送低电量报警
    */
    static volatile u8 is_first_time_to_send = 0;

    if (is_sent_low_bat_alert)
    {
        cnt++;
        if ((is_first_time_to_send && cnt >= (u16)3 * 1000) ||
            (cnt >= SEND_LOW_BAT_TIME_PERIOD))
        {
            if (is_first_time_to_send)
            {
                is_first_time_to_send = 0;
            }

            cnt = 0;
            is_send_low_bat_repeatedly_enable = 1;
        }
    }
    else
    {
        cnt = 0;

        // 低电量报警标志没有使能，给标志位置一
        is_first_time_to_send = 1;
    }
}

void batttery_monitor_1ms_isr(void)
{
    static volatile u16 cnt_1ms = 0;

    if (cnt_1ms < ((u16)-1))
    {
        cnt_1ms++;
    }

    if (is_in_charging)
    {
        // 正在充电，累计充电时间
        if (cnt_1ms >= 1000)
        {
            cnt_1ms = 0;
            if (charge_time_cnt < ((u32)-1))
            {
                charge_time_cnt++;
            }
        }
    }
    else if (is_in_charging == 0 &&
             (ble_ic.is_working ||
              led_ctl.status != LED_STATUS_OFF ||
              is_in_discharging))
    {
        // 正在放电，累计放电时间
        if (cnt_1ms >= 1000)
        {
            cnt_1ms = 0;
            if (discharge_time_cnt < ((u32)-1))
            {
                discharge_time_cnt++;
            }
        }
    }
    else
    {
        cnt_1ms = 0;
    }
}

/**
 * @brief 初始化电池充电时间
 *      1. 在低功耗唤醒后调用
 *      2. 在第一次上电后调用
 *
 */
void bat_charge_time_cnt_update(u16 voltage_mv)
{
    static u8 is_initialized = 0;
    if (0 == is_initialized)
    {
        // 没有初始化过 充电时间，直接根据当前电池电压，大致给出充电时间
        is_initialized = 1;
        // 根据黄白灯一起亮的数据来初始化充电时间
        if (voltage_mv >= BAT_WY_4LED_VOLTAGE)
        {
            // 电池电压大于4个灯对应的电压
            charge_time_cnt = BAT_CHARGE_3LED_TIME;
        }
        else if (voltage_mv >= BAT_WY_3LED_VOLTAGE)
        {
            // 电池电压大于3个灯对应的电压
            charge_time_cnt = BAT_CHARGE_3LED_TIME;
        }
        else if (voltage_mv >= BAT_WY_2LED_VOLTAGE)
        {
            // 电池电压大于2个灯对应的电压
            charge_time_cnt = BAT_CHARGE_2LED_TIME;
        }
        else if (voltage_mv >=
                 (BAT_WY_LOW_WARN_VOLTAGE + BAT_WY_DEAD_ZONE_VOLTAGE))
        {
            // 电池电压大于低电量报警电压
            charge_time_cnt = BAT_CHARGE_1LED_TIME;
        }
        else
        {
            charge_time_cnt = 0;
        }

        return;
    }

    // 如果已经初始化过，根据之前保存的电池电量指示灯状态，给充电时间赋值
    switch (led_bat_lev_sta)
    {
    case LED_BAT_LEV_STA_ALERT:
        charge_time_cnt = 0;
        break;

    case LED_BAT_LEV_STA_1:
        // 之前点亮了第一颗指示灯
        charge_time_cnt = BAT_CHARGE_1LED_TIME;
        break;

    case LED_BAT_LEV_STA_2:
        charge_time_cnt = BAT_CHARGE_2LED_TIME;
        break;

    case LED_BAT_LEV_STA_3:
        charge_time_cnt = BAT_CHARGE_3LED_TIME;
        break;

    case LED_BAT_LEV_STA_4:
        charge_time_cnt = BAT_CHARGE_3LED_TIME;
        break;
    }
}

/**
 * @brief 初始化电池放电时间
 *      1. 在低功耗唤醒后调用
 *      2. 在第一次上电后调用
 *
 */
void bat_discharge_time_cnt_update(u16 voltage_mv)
{
    static u8 is_initialized = 0;
    if (0 == is_initialized)
    {
        is_initialized = 1;

        // 根据黄白灯一起亮的数据来初始化放电时间
        if (voltage_mv >= BAT_WY_4LED_VOLTAGE)
        {
            // 电池电压大于4个灯对应的电压
            discharge_time_cnt = 0;
        }
        else if (voltage_mv >= BAT_WY_3LED_VOLTAGE)
        {
            // 电池电压大于3个灯对应的电压
            discharge_time_cnt = BAT_WY_3LED_TIME;
        }
        else if (voltage_mv >= BAT_WY_2LED_VOLTAGE)
        {
            // 电池电压大于2个灯对应的电压
            discharge_time_cnt = BAT_WY_2LED_TIME;
        }
        else if (voltage_mv >=
                 (BAT_WY_LOW_WARN_VOLTAGE + BAT_WY_DEAD_ZONE_VOLTAGE))
        {
            // 电池电压大于低电量报警电压
            discharge_time_cnt = BAT_WY_1LED_TIME;
        }
        else
        {
            discharge_time_cnt = BAT_WY_LOW_WARN_TIME;
        }

        return;
    }

    // 如果已经初始化过，根据之前保存的电池电量指示灯状态，给放电时间赋值
    switch (led_bat_lev_sta)
    {
    case LED_BAT_LEV_STA_ALERT:
        discharge_time_cnt = BAT_WY_LOW_WARN_TIME;
        break;

    case LED_BAT_LEV_STA_1:
        // 之前点亮了第一颗指示灯
        discharge_time_cnt = BAT_WY_1LED_TIME;
        break;

    case LED_BAT_LEV_STA_2:
        discharge_time_cnt = BAT_WY_2LED_TIME;
        break;

    case LED_BAT_LEV_STA_3:
        discharge_time_cnt = BAT_WY_3LED_TIME;
        break;

    case LED_BAT_LEV_STA_4:
        discharge_time_cnt = 0;
        break;
    }
}

// 修改后的电池监控处理函数
void battery_monitor_handle(void)
{
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
             led_ctl.status == LED_STATUS_OFF &&
             0 == is_in_discharging) // 充电ic没有输出放电的信号
    {
        led_bat_level_sta = LED_BAT_LEVEL_STA_IDLE;

        // 关机后（不在工作），需要在这里清空对应标志位
    }
    // 不在充电，但是蓝牙ic或者灯光打开，根据电池电量来点亮对应指示灯
    else if (is_in_charging == 0 &&
             (ble_ic.is_working ||
              led_ctl.status != LED_STATUS_OFF ||
              is_in_discharging))
    {
        led_bat_level_sta = LED_BAT_LEVEL_STA_DISCHARGE;

        // 到了关机对应的电压
        if (avg_voltage_mv <= BATTERY_EMPTY_VOLTAGE)
        {
#if USER_DEBUG_ENABLE
            printf("detect bat power empty\n");
            // printf("led_ctl.status == %u\n", (u16)led_ctl.status);
            // printf("ble_ic.is_working == %u\n", (u16)ble_ic.is_working);
#endif

            // 关闭灯光
            if (led_ctl.status != LED_STATUS_OFF)
            {
                led_status_set(LED_STATUS_OFF);
            }

            /*
                关闭蓝牙，从低电量提示到低电量关机。
                这个时间段会打开蓝牙，进行低电量提示。
                到了低电量关机，这里需要能够重复进来，一直给蓝牙发送数据，确保蓝牙关机
            */
            if (ble_ic.is_working)
            {
                uart_data_send_cmd(UART_SEND_CMD_LOW_POWER_SHUTDOWN);
                // delay_ms(5000); // 这里要等蓝牙播报完成
                delay_ms(30);
                ble_ic_disable_pre();
                delay_ms(100);
            }

            is_in_low_bat_alert = 0; // 关机，取消低电量报警
        }
        /*
            低电量，并且没有发送过低电量报警，要打开蓝牙，发送低电量报警
            电池电量在关机电压和低电量阈值之间，才打开蓝牙
        */
        else if (is_sent_low_bat_alert == 0 &&
                 /* 扩展低电量触发阈值，使用模式配置中的低电警告阈值（参考客户要求） */
                 avg_voltage_mv <= BATTERY_LOW_WARNING_VOLTAGE)
        {
            // u8 i; // 循环计数值，控制连续发送低电量报警的次数
#if USER_DEBUG_ENABLE
            // printf("detect low power\n");
#endif
            if (ble_ic.is_working == 0)
            {
                ble_ic_enable();
            }

            uart_data_send_cmd(UART_SEND_CMD_LOW_POWER_WARNING);
            is_sent_low_bat_alert = 1;
            is_in_low_bat_alert = 1;
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
}