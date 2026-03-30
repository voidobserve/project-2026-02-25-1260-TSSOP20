#include "battery_monitor.h"
#include "user_include.h"

// 电池模型参数
#define BATTERY_FULL_VOLTAGE 4200        // 满电电压 (mV)
#define BATTERY_EMPTY_VOLTAGE 3300       // 空电电压 (mV)
#define BATTERY_LOW_WARNING_VOLTAGE 3600 // 低电量警告电压 (mV)

volatile u8 is_send_low_battery_enable = 0;

static volatile u8 is_bat_vol_buff_add_enable = 0;     // 是否允许往电池电压数组中放入数据
static volatile u8 is_bat_vol_buff_get_avg_enable = 0; // 是否允许往电池电压数组中获取平均值

volatile bat_vol_update_sta_t bat_vol_update_sta = BAT_VOL_UPDATE_STA_IDLE;
static volatile u16 bat_vol_update_cnt = 0;

static volatile u16 bat_vol_history_buff[VOLTAGE_HISTORY_SIZE] = {0};
static volatile u8 bat_vol_history_buff_idx = 0;

volatile u8 bat_percent = 0;




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
}

void send_low_battery_timer_callback(void)
{
    static u32 cnt = 0;
    cnt++;
    if (cnt >= (u32)30 * 1000)
    {
        is_send_low_battery_enable = 1;
        cnt = 0;
    }
}

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

// 从系统获取当前时间戳 (需要在其他地方实现)
extern u32 get_system_tick_count(void); // 假设这个函数提供系统运行时间(毫秒)
volatile u32 sys_time;
void sys_time_add(void)
{
    sys_time++;
}

u32 get_system_tick_count(void)
{
    return sys_time;
}

// 修改后的电池监控处理函数
void battery_monitor_handle(void)
{
    static volatile u16 voltage_mv = 0;     //
    static volatile u16 max_voltage_mv = 0; // 存放一段时间内采集到的最大电压值（只在采集使用，不能作为最终的判断使用）
    static volatile u16 avg_voltage_mv = 0;
    u8 percent = 0;
    u8 last_percent = 0;

    u16 adc_val = 0;
    u16 cur_voltage_mv = 0; // 存放当前采集到的电压值

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

            // 初始化放电模型
            bat_vol_history_buff_init(voltage_mv);
            avg_voltage_mv = bat_vol_history_buff_get_avg();
            percent = get_battery_percentage_by_voltage(avg_voltage_mv);
            last_percent = percent;
            bat_percent = percent; // 初始化全局变量，电池电量百分比
        }

        // 采集一段时间的电压值，取其中最大的值作为电池电压
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
    }

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
        printf("avg_voltage_mv == %u\n", avg_voltage_mv);

        percent = get_battery_percentage_by_voltage(avg_voltage_mv);
        printf("percent == %u\n", (u16)percent);

        is_bat_vol_buff_get_avg_enable = 0;
    }

    // 充电中，percent大于等于last_percent，不让percent小于last_percent
    if (is_in_charging_by_charger || is_in_charging_by_solar_panel)
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

    bat_percent = percent; 

    // 测试时使用，充电动画：
    if (is_in_charging_by_charger)
    {
        LED_100_PERCENT_ON();
        LED_75_PERCENT_ON();
        LED_50_PERCENT_ON();
        LED_25_PERCENT_ON();
    }
    else
    {
        LED_100_PERCENT_OFF();
        LED_75_PERCENT_OFF();
        LED_50_PERCENT_OFF();
        LED_25_PERCENT_OFF();
    }

    /*
        无指示灯亮起：电压 < 3.3V（低于最低电量，需要关机）
        1个指示灯亮起：3.3V ≤ 电压 < 3.6V（低电量状态）
        2个指示灯亮起：3.6V ≤ 电压 < 4.0V（中等电量状态）
        3个指示灯亮起：4.0V ≤ 电压 < 4.2V（较高电量状态）
        4个指示灯全亮：电压 ≥ 4.2V（满电状态）
    */

}