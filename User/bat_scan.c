#include "bat_scan.h"
#include "adc.h"
#include "led.h"
#include "charge_det.h"
#include "battery_monitor.h"
#include "led_bat_lev.h"

#include "user_config.h"

/*
	控制一段时间内采集电池电压的状态机
*/
static volatile bat_vol_update_sta_t bat_vol_update_sta = BAT_VOL_UPDATE_STA_IDLE;
static volatile u16 bat_vol_update_cnt = 0;

// 记录一段时间内采集到的电压值：
static volatile u16 bat_vol_history_buff[VOLTAGE_HISTORY_SIZE] = {0};
static volatile u8 bat_vol_history_buff_idx = 0;
// ====================================================================

#if 0
static volatile u16 max_voltage_mv = 0; // 存放一段时间内采集到的最大电压值（只在采集使用，不能作为最终的判断使用）
#endif
static volatile u16 min_voltage_mv = ((u16)-1); // 存放一段时间内采集到的最小电压值（只在采集使用，不能作为最终的判断使用）
volatile u16 voltage_mv_global = 0;				//

// 采集电池电压的定时计数器
static volatile u32 bat_avg_vol_scan_period_cnt = 0;
// 最后得到的、稳定的电池电压
volatile u16 avg_voltage_mv = 0; //

void bat_get_avg_vol_period_add(void)
{
	if (bat_avg_vol_scan_period_cnt < ((u32)-1))
	{
		bat_avg_vol_scan_period_cnt++;
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

// 根据ad值，转换成对应的电池电压值(单位：mV)
u16 get_battery_voltage_by_adc(u16 adc_val)
{
	u16 voltage_mv = ADC_TO_BATTERY_VOLTAGE_MV(adc_val);

#if 1
	// printf("voltage_mv == %u\n", voltage_mv);
	if (voltage_mv > 60)
	{
		// 这里做电压补偿
		voltage_mv -= 60;
	}

#if USER_DEBUG_ENABLE
	// printf("voltage_mv == %u\n", voltage_mv);
#endif
#endif

#if 0
    if (is_in_charging)
    {
        // 充电时，减去 0.1 V，作为补偿。如果电压接近 4.2V 时，不用补偿
        if ((voltage_mv <= 4000) && voltage_mv > 100)
        {
            voltage_mv -= 100;
        }
    }
#endif

	return voltage_mv;
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

static void bat_vol_history_buff_add(u16 voltage_mv)
{
	bat_vol_history_buff[bat_vol_history_buff_idx] = voltage_mv;
	// bat_vol_history_buff_idx = (bat_vol_history_buff_idx + 1) % VOLTAGE_HISTORY_SIZE;
	bat_vol_history_buff_idx++;
	if (bat_vol_history_buff_idx >= VOLTAGE_HISTORY_SIZE)
	{
		bat_vol_history_buff_idx = 0;
	}
}

// 获取缓冲区中所有元素的平均值
u16 bat_vol_history_buff_get_avg(void)
{
	volatile u8 i;
	volatile u32 ret = 0;
	for (i = 0; i < VOLTAGE_HISTORY_SIZE; i++)
	{
		ret += bat_vol_history_buff[i];
	}

#if USER_DEBUG_ENABLE
	// printf("ret == %lu\n", (u32)ret);
	// printf("ret / VOLTAGE_HISTORY_SIZE == %u\n", (u16)(ret / VOLTAGE_HISTORY_SIZE));
#endif

	return (u16)((u32)ret / VOLTAGE_HISTORY_SIZE);
}

/**
 * @brief 更新电池电压
 * 		由adc中断服务函数调用
 * 		函数内部会更新最新得到的电池电压
 *
 */
void battery_voltage_update_by_isr(void)
{
	static volatile u8 is_initialized = 0;
	volatile u16 adc_val = 0;
	volatile u16 cur_voltage_mv = 0; // 存放当前采集到的电压值

	// 获取AD值（ad值有更新才获取）
	if (adc_get_update_flag(ADC_CHANNEL_SEL_BAT_DET))
	{
		adc_clear_update_flag(ADC_CHANNEL_SEL_BAT_DET);
		adc_val = adc_get_val(ADC_CHANNEL_SEL_BAT_DET);
		// printf("bat adc val == %u\n", adc_val);
		cur_voltage_mv = get_battery_voltage_by_adc(adc_val);
		// printf("cur_voltage_mv == %u\n", cur_voltage_mv);

		if (0 == is_initialized)
		{
			is_initialized = 1;
			// 如果还没有采集过电池电压，直接将第一次采集到的ad值转换成电池电压
			voltage_mv_global = cur_voltage_mv;
			avg_voltage_mv = cur_voltage_mv;

			bat_vol_history_buff_init(avg_voltage_mv);
			// 刚上电时，直接根据对应的电池电压，给充电时间、放电时间进行赋值
			led_bat_lev_init_by_vol(avg_voltage_mv);
			bat_charge_time_cnt_update(avg_voltage_mv);
			bat_discharge_time_cnt_update(avg_voltage_mv);

			/*
				第一次上电，初始化完成后，也给该标志位置一
				防止第一次上电，没有进入低功耗就开机，
				电压突然从4.0V拉低到3.5V，导致电池电量挡位误判断
			*/ 
			is_low_power_wakeup_initialize_enable = 1; 

#if USER_DEBUG_ENABLE
			printf("bat monitor init\n");
			printf("avg_voltage_mv == %u\n", avg_voltage_mv);
#endif
		}

#if 0
		// 采集一段时间的电压值，取其中最大的值作为电池电压		
		if (bat_vol_update_sta == BAT_VOL_UPDATE_STA_COMPLETED)
		{
			// 如果已经采集了一段时间的电压值，则将 voltage_mv_global 赋值为 max_voltage_mv
			voltage_mv_global = max_voltage_mv;

			max_voltage_mv = 0; // 清零，准备下一轮采集
			bat_vol_update_sta = BAT_VOL_UPDATE_STA_IDLE;

			// 输出计算结果 (调试用)
			// printf("voltage_mv_global: %umV\n", voltage_mv_global);
		}
		else if (bat_vol_update_sta == BAT_VOL_UPDATE_STA_CAPTURING)
		{
			// 如果还在采集时间内，更新采集到的最大电压值 max_voltage_mv
			if (max_voltage_mv < cur_voltage_mv)
			{
				max_voltage_mv = cur_voltage_mv;
			}
		}
#endif

#if 0
		// 采集一段时间的电压值，取其中最小的值作为电池电压
		if (bat_vol_update_sta == BAT_VOL_UPDATE_STA_COMPLETED)
		{
			// 如果已经采集了一段时间的电压值，则将 voltage_mv_global 赋值为 min_voltage_mv
			voltage_mv_global = min_voltage_mv;

			min_voltage_mv = ((u16)-1); // 清零，准备下一轮采集
			bat_vol_update_sta = BAT_VOL_UPDATE_STA_IDLE;

			// 输出计算结果 (调试用)
			// printf("voltage_mv_global: %umV\n", voltage_mv_global);
		}
		else if (bat_vol_update_sta == BAT_VOL_UPDATE_STA_CAPTURING)
		{
			// 如果还在采集时间内，更新采集到的最小电压值 min_voltage_mv
			if (min_voltage_mv > cur_voltage_mv)
			{
				min_voltage_mv = cur_voltage_mv;
			}
		}
#endif

		voltage_mv_global = cur_voltage_mv;
	}
}

/**
 * @brief 定期给存放电池电压的滑动平均数组添加数据
 * 		由 1ms 定时器调用
 *
 */
void bat_vol_history_buff_add_timer_callback(void)
{
	static volatile u32 cnt = 0;
	cnt++;
	if (cnt >= BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER)
	{
		cnt = 0;
		// 每隔一段时间，将采集到的电压值放入数组
		bat_vol_history_buff_add(voltage_mv_global);

#if USER_DEBUG_ENABLE
		printf("voltage_mv_global == %u\n", voltage_mv_global);
#endif
	}
}

void bat_scan(void)
{
	if (voltage_mv_global == 0)
	{
		return; // 尚未获取到电压值，不执行以下操作
	}

	if (is_low_power_wakeup_initialize_enable)
	{
		// 低功耗唤醒后的一段时间内，不采集电池电压，等电压稳定之后再采集
		return;
	}

	// 每隔一段时间，将数组中的数据进行平均
	if (bat_avg_vol_scan_period_cnt >= BATTERY_VOLTAGE_UPDATE_PERIOD_IN_BUFFER_EXTRACT)
	{
		bat_avg_vol_scan_period_cnt = 0;
		avg_voltage_mv = bat_vol_history_buff_get_avg();
#if USER_DEBUG_ENABLE
		printf("avg_voltage_mv == %u\n", (u16)avg_voltage_mv);
#endif
	}
}
