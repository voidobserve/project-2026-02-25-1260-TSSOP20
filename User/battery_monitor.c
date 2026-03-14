#include "battery_monitor.h"
#include "user_include.h"

#define SLIP_AVER_BUFF_LEN 200
static u8 slip_avg_buff[SLIP_AVER_BUFF_LEN] = {0}; // 修复数组初始化语法
static u8 slip_avg_buff_index = 0;

// USER_TO_DO 关机之后需要清空这个标志位：
volatile u8 is_send_low_battery = 0; // 是否发送过低电压的提示
volatile u8 is_low_battery = 0;      // 是否处于低电量状态

// 添加滞回比较所需的电量状态记录
static u8 current_battery_level = 100; // 当前电量百分比，用于滞回比较

volatile u8 is_battery_monitor_time_comes = 0; // 控制函数调用周期的变量

/*
    USER_TO_DO  后续的版本还需要再优化

    每个 xx S，强制刷新电池电量百分比
*/
#define BATTERY_LEVEL_REFRESH_FORCE_INTERVAL ((u32)60 * 1000)
volatile u8 is_refresh_battery_level = 0;    // 是否要强制刷新电池电量百分比
volatile u8 is_battery_level_add_enable = 0; // 是否允许电量百分比增加（只有在充电时才允许电池电量百分比增加，防止电量百分比跳动）

volatile u8 stable_bat_percent = 0; // 稳定的电量百分比（经滞回处理）


volatile u8 is_send_low_battery_enable = 0;

void send_low_battery_timer_callback(void)
{
    static u32 cnt = 0;
    cnt++;
    if (cnt >= (u32) 30 * 1000)
    {
        is_send_low_battery_enable = 1;
        cnt = 0;
    }
}

void refresh_battery_level_timer_callback(void)
{
    static u32 cnt = 0;
    cnt++;
    if (cnt >= BATTERY_LEVEL_REFRESH_FORCE_INTERVAL)
    {
        is_refresh_battery_level = 1;
        cnt = 0;
    }
}

void slip_avg_buff_init(u8 val)
{
    u8 i;
    for (i = 0; i < SLIP_AVER_BUFF_LEN; i++)
    {
        slip_avg_buff[i] = val;
    }
}

void slip_avg_buff_put(u8 val)
{
    slip_avg_buff[slip_avg_buff_index] = val;
    slip_avg_buff_index++;
    if (slip_avg_buff_index >= SLIP_AVER_BUFF_LEN)
    {
        slip_avg_buff_index = 0;
    }
}

u8 slip_avg_get_filtered_val(void)
{
    u8 i;
    u32 ret = 0;
    for (i = 0; i < SLIP_AVER_BUFF_LEN; i++)
    {
        ret += slip_avg_buff[i];
    }

    ret /= SLIP_AVER_BUFF_LEN;
    return (u8)ret;
}

// 供外部调用
u8 bat_percent_get(void)
{
    // 获取滑动平均后的电池电量百分比
    return slip_avg_get_filtered_val();
}

// 根据电压值计算电池电量百分比 (0-100)
u8 get_battery_percentage_by_voltage(u16 voltage_mv)
{
    if (voltage_mv >= BATTERY_VOLTAGE_100_PERCENT)
    {
        return 100;
    }
    else if (voltage_mv <= BATTERY_VOLTAGE_0_PERCENT)
    {
        return 0;
    }

    // 线性插值计算百分比
    if (voltage_mv >= BATTERY_VOLTAGE_75_PERCENT)
    {
        // 100% - 75% 区间
        return 75 + ((u32)(voltage_mv - BATTERY_VOLTAGE_75_PERCENT) * 25) /
                        (BATTERY_VOLTAGE_100_PERCENT - BATTERY_VOLTAGE_75_PERCENT);
    }
    else if (voltage_mv >= BATTERY_VOLTAGE_50_PERCENT)
    {
        // 75% - 50% 区间
        return 50 + ((u32)(voltage_mv - BATTERY_VOLTAGE_50_PERCENT) * 25) /
                        (BATTERY_VOLTAGE_75_PERCENT - BATTERY_VOLTAGE_50_PERCENT);
    }
    else if (voltage_mv >= BATTERY_VOLTAGE_25_PERCENT)
    {
        // 50% - 25% 区间
        return 25 + ((u32)(voltage_mv - BATTERY_VOLTAGE_25_PERCENT) * 25) /
                        (BATTERY_VOLTAGE_50_PERCENT - BATTERY_VOLTAGE_25_PERCENT);
    }
    else
    {
        // 25% - 0% 区间
        return ((u32)(voltage_mv - BATTERY_VOLTAGE_0_PERCENT) * 25) /
               (BATTERY_VOLTAGE_25_PERCENT - BATTERY_VOLTAGE_0_PERCENT);
    }
}

// 使用滞回比较获取稳定的电量百分比
static u8 get_battery_percentage_with_hysteresis(u8 raw_percent)
{
// 定义滞回阈值，防止在临界点附近抖动
#define HYSTERESIS_THRESHOLD 20 // 滞回阈值为 x%

    u8 result = current_battery_level;

    // 只有当新读数与当前值的差异超过滞回阈值时才更新
    if (raw_percent > current_battery_level + HYSTERESIS_THRESHOLD)
    {
        // 电量上升超过滞回阈值，更新到新的更高值
        result = raw_percent;
    }
    else if (raw_percent < current_battery_level - HYSTERESIS_THRESHOLD)
    {
        // 电量下降超过滞回阈值，更新到新的更低值
        result = raw_percent;
    }
    // 否则保持当前值不变，防止抖动

    // 更新当前电量状态
    current_battery_level = result;

    return result;
}

// // 根据ADC值获取电池电量等级
// battery_level_t get_battery_level_by_adc(u16 adc_val)
// {
//     u16 voltage_mv = ADC_TO_BATTERY_VOLTAGE_MV(adc_val);
//     return get_battery_level_by_voltage(voltage_mv);
// }

// 根据ADC值计算电池电量百分比
u8 get_battery_percentage_by_adc(u16 adc_val)
{
    u16 voltage_mv = ADC_TO_BATTERY_VOLTAGE_MV(adc_val);
    // printf("voltage_mv == %u\n", voltage_mv); // 打印转换好的电压值

    if (ble_ic.is_working != 0 || led_ctl.status != LED_STATUS_OFF)
    {
        // 蓝牙ic开着，或者灯亮着，加0.3V作为补偿
        voltage_mv += 300;
    }

    return get_battery_percentage_by_voltage(voltage_mv);
}

// 电池电压划分建议说明
/*
电池电压范围：2.5V - 4.2V 的划分方案：

1. 电压区间划分（考虑锂电池特性）：
   - 100% (满电):    4.20V
   - 75% (高电量):   3.85V
   - 50% (中电量):   3.60V
   - 25% (低电量):   3.20V
   - 0%  (耗尽):     2.50V

2. 划分依据：
   - 锂电池放电曲线特性：电压下降不是完全线性的
   - 实际使用经验：大部分使用时间集中在3.2V-4.2V之间
   - 安全考虑：避免过度放电损坏电池

3. ADC值对应关系（使用内部2.0V参考电压，1/5分压）：
   - 4.20V → ADC值约 2048
   - 3.85V → ADC值约 1876
   - 3.60V → ADC值约 1755
   - 3.20V → ADC值约 1560
   - 2.50V → ADC值约 1221

4. 使用建议：
   - 可以根据实际电池型号调整这些电压值
   - 建议添加滞回比较防止电量显示抖动
   - 低电量时应该给出警告提示
*/

void battery_monitor_init(void)
{
}

void battery_monitor_handle(void)
{
    static u16 adc_val = 0;
    static u8 is_initialized = 0;
    u8 bat_percent = 0; // 电池电量百分比

    // 获取AD值
    if (adc_get_update_flag(ADC_CHANNEL_SEL_BAT_DET))
    {
        adc_clear_update_flag(ADC_CHANNEL_SEL_BAT_DET);
        adc_val = adc_get_val(ADC_CHANNEL_SEL_BAT_DET);
        // printf("bat adc val == %u\n", adc_val);
    }

    // 获取原始电量百分比
    bat_percent = get_battery_percentage_by_adc(adc_val);
    // printf("bat_percnent == %u\n", (u16)bat_percent);

    if (0 == is_initialized)
    {
        slip_avg_buff_init(bat_percent);
        is_initialized = 1;
    }

    slip_avg_buff_put(bat_percent);
    bat_percent = slip_avg_get_filtered_val();
    // printf("bat_percent = %u\n", (u16)bat_percent);

    // 使用滞回比较算法处理电量百分比，防止显示抖动
    stable_bat_percent = get_battery_percentage_with_hysteresis(bat_percent);

    if (is_refresh_battery_level)
    {
        stable_bat_percent = slip_avg_get_filtered_val();
        is_refresh_battery_level = 0;
    }

    // printf("stable_bat_percent = %u\n", (u16)stable_bat_percent);

    // USER_TO_DO
    /*
        充电时、放电时、太阳能一侧的电压比电池电压还大时，才进行电池电量显示
    */
    if ((led_ctl.status == LED_STATUS_OFF) &&
        (ble_ic.is_working == 0) &&
        (is_in_charging_by_charger == 0) &&
        (is_in_charging_by_solar_panel == 0))
    {
        // 不需要电量显示的场合，把电量指示灯全部关闭
        LED_25_PERCENT_OFF();
        LED_50_PERCENT_OFF();
        LED_75_PERCENT_OFF();
        LED_100_PERCENT_OFF();
        return;
    }

    // if (stable_bat_percent <= 25 && is_send_low_battery == 0)
    // {
    //     // 低电量，并且没有发送低电量的提示
    //     uart_data_send_cmd(UART_SEND_CMD_LOW_POWER_WARNING);
    //     is_send_low_battery = 1;
    //     is_low_battery = 1;
    // }

    if (stable_bat_percent <= 25 && is_send_low_battery_enable)
    {
        // 低电量，并且没有发送低电量的提示
        uart_data_send_cmd(UART_SEND_CMD_LOW_POWER_WARNING);
        // is_send_low_battery = 1;
        // is_low_battery = 1;
        is_send_low_battery_enable = 0;
    }

    if (stable_bat_percent <= 0)
    {
        // 低电量关机

        // 给蓝牙ic发送低电量关机
        uart_data_send_cmd(UART_SEND_CMD_LOW_POWER_SHUTDOWN);

        // 关闭电量指示灯
        LED_25_PERCENT_OFF();
        LED_50_PERCENT_OFF();
        LED_75_PERCENT_OFF();
        LED_100_PERCENT_OFF();

        // 关闭驱动的灯光
        led_ctl.status = LED_STATUS_OFF;
        LED_WHITE_OFF();
        LED_YELLOW_OFF();
        P02 = 1;
        P01 = 1;

        is_send_low_battery = 0;
        is_low_battery = 1;
        // 之后在串口等待蓝牙ic回复已经关闭功放的数据，再关闭蓝牙
        return;
    }

    if (stable_bat_percent >= 30)
    {
        is_send_low_battery = 0;
    }

    if (stable_bat_percent >= 25)
    {
        LED_25_PERCENT_ON();
    }

    if (stable_bat_percent >= 50)
    {
        LED_50_PERCENT_ON();
    }
    else
    {
        LED_50_PERCENT_OFF();
    }

    if (stable_bat_percent >= 75)
    {
        LED_75_PERCENT_ON();
    }
    else
    {
        LED_75_PERCENT_OFF();
    }

    if (stable_bat_percent >= 100)
    {
        LED_100_PERCENT_ON();
    }
    else
    {
        LED_100_PERCENT_OFF();
    }
}