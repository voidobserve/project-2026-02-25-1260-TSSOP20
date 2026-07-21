#include "led_bat_lev.h"
#include "led.h"

#include "bat_scan.h"
#include "charge_det.h"
#include "bluetooth_ic_handle.h"
#include "battery_monitor.h"

#include "user_config.h"

// 电池电量指示灯挡位
volatile led_bat_lev_t led_bat_lev = LED_BAT_LEV_OFF;
// 电池电量指示灯状态
volatile led_bat_lev_sta_t led_bat_lev_sta = LED_BAT_LEV_STA_IDLE;

// 充电时，控制电池电量指示灯：
// 控制电池电量指示灯的状态：
static volatile u8 led_charge_anim_phase = 0; // 充电开始的动画阶段，0：灯光全灭，1：开始点亮第一个灯
static volatile u16 led_charge_anim_cnt = 0;  // 充电动画计数

/**
 * @brief 根据电压值初始化电池电量指示灯挡位
 *
 * 		目前只在第一次上电后调用
 *
 * @note 刚接电池时，根据电池的放电曲线对应的电压来给电量指示灯挡位作划分
 */
void led_bat_lev_init_by_vol(u16 voltage_mv)
{
	if (voltage_mv < BATTERY_EMPTY_VOLTAGE)
	{
		led_bat_lev = LED_BAT_LEV_OFF;
	}
	else if (voltage_mv < BATTERY_LOW_WARNING_VOLTAGE)
	{
		led_bat_lev = LED_BAT_LEV_ALERT;
	}
	else if (voltage_mv < LED_BAT_LEV_25_PERCENT_MAP_TO_VOLTAGE_DURING_DISCHARGING)
	{
		led_bat_lev = LED_BAT_LEV_1;
	}
	else if (voltage_mv < LED_BAT_LEV_50_PERCENT_MAP_TO_VOLTAGE_DURING_DISCHARGING)
	{
		led_bat_lev = LED_BAT_LEV_2;
	}
	else if (voltage_mv < LED_BAT_LEV_75_PERCENT_MAP_TO_VOLTAGE_DURING_DISCHARGING)
	{
		led_bat_lev = LED_BAT_LEV_3;
	}
	else
	{
		led_bat_lev = LED_BAT_LEV_4;
	}
}

void led_bat_lev_handle(void)
{
	if (is_in_charging &&
		(led_bat_lev_sta != LED_BAT_LEV_STA_CHARGE_BEGIN &&
		 led_bat_lev_sta != LED_BAT_LEV_STA_CHARGE_BEGIN_ANIM &&
		 led_bat_lev_sta != LED_BAT_LEV_STA_CHARGING &&
		 led_bat_lev_sta != LED_BAT_LEV_STA_CHARGE_END))
	{
		// 检测到正在充电，电池电量指示灯却不在充电对应的状态，则进入充电开始的状态
		led_bat_lev_sta = LED_BAT_LEV_STA_CHARGE_BEGIN;
	}
	else if (is_in_charging == 0 &&
			 ble_ic.is_working == 0 &&
			 led_ctl.status == LED_STATUS_OFF &&
			 0 == is_in_discharging) // 充电ic没有输出放电的信号
	{
		/*
			不在充电
			蓝牙ic没有工作
			主灯光没有打开
			充电IC没有输出放电的信号
			这些条件都满足时，电池电量指示灯进入待机状态
		*/
		led_bat_lev_sta = LED_BAT_LEV_STA_IDLE;
	}
	else if (is_in_charging == 0 &&
			 (ble_ic.is_working ||
			  led_ctl.status != LED_STATUS_OFF ||
			  is_in_discharging))
	{
		if (led_bat_lev_sta != LED_BAT_LEV_STA_DISCHARGE &&
			led_bat_lev_sta != LED_BAT_LEV_STA_ALERT)
		{
			// 不在放电，也不在放电低电量提示状态，才进入放电状态
			led_bat_lev_sta = LED_BAT_LEV_STA_DISCHARGE;
		}
	}
}

// 放电时，电池电量指示灯的处理函数
static void led_bat_lev_sta_discharge_handle(void)
{
	static volatile u32 jump_down_cnt_ms = 0; // 跳级向下的去抖计时（ms）

	volatile u8 target_bat_lev = LED_BAT_LEV_4; // 目标电量等级

	if (led_ctl.status == LED_STATUS_YELLOW ||
		led_ctl.status == LED_STATUS_WHITE ||
		led_ctl.status == LED_STATUS_RED_BLUE_FLASH)
	{
		// 只有一种灯亮（single light）
		if (avg_voltage_mv < BAT_SINGLE_LIGHT_LOW_WARN_VOLTAGE)
		{
			target_bat_lev = LED_BAT_LEV_ALERT;
		}
		else if (avg_voltage_mv < BAT_SINGLE_LIGHT_1LED_VOLTAGE)
		{
			target_bat_lev = LED_BAT_LEV_1;
		}
		else if (avg_voltage_mv < BAT_SINGLE_LIGHT_2LED_VOLTAGE)
		{
			target_bat_lev = LED_BAT_LEV_2;
		}
		else if (avg_voltage_mv < BAT_SINGLE_LIGHT_3LED_VOLTAGE)
		{
			target_bat_lev = LED_BAT_LEV_3;
		}
		else if (avg_voltage_mv > BAT_SINGLE_LIGHT_3LED_VOLTAGE)
		{
			target_bat_lev = LED_BAT_LEV_4;
		}
		else
		{
			// REVIEW
			// 特殊情况
		}
	}
	else if (led_ctl.status == LED_STATUS_WHITE_YELLOW)
	{
		// 黄白灯都亮
		if (avg_voltage_mv < BAT_WY_LOW_WARN_VOLTAGE)
		{
			target_bat_lev = LED_BAT_LEV_ALERT;
		}
		else if (avg_voltage_mv < BAT_WY_1LED_VOLTAGE)
		{
			target_bat_lev = LED_BAT_LEV_1;
		}
		else if (avg_voltage_mv < BAT_WY_2LED_VOLTAGE)
		{
			target_bat_lev = LED_BAT_LEV_2;
		}
		else if (avg_voltage_mv < BAT_WY_3LED_VOLTAGE)
		{
			target_bat_lev = LED_BAT_LEV_3;
		}
		else
		{
			target_bat_lev = LED_BAT_LEV_4;
		}
	}

#if USER_DEBUG_ENABLE

	{
		static u16 cnt = 0;
		cnt++;
		if (cnt >= 1000)
		{
			cnt = 0;
			printf("target_bat_lev == %u\n", (u16)target_bat_lev);
		}
	}
#endif

	/*
		判断当前显示的电池电量等级是否跟目前电量等级相等
		如果不相等，
			- 判断挡位相差是否在1以内
				如果在1以内，判断放电时间是否满足挡位对应的事件
				如果不再1以内，需要跳级
	*/
	if (led_bat_lev > target_bat_lev)
	{
		if (led_bat_lev - target_bat_lev <= 1)
		{
			/*
				当前显示的电池电量与目标电量只相差1以内，
				判断是否到了对应的放电时间
			*/
			jump_down_cnt_ms = 0;

			if (led_ctl.status == LED_STATUS_YELLOW ||
				led_ctl.status == LED_STATUS_WHITE ||
				led_ctl.status == LED_STATUS_RED_BLUE_FLASH)
			{
				// 单色灯
				switch (target_bat_lev)
				{
				case LED_BAT_LEV_3:
					if (discharge_time_cnt >= BAT_SINGLE_LIGHT_3LED_TIME)
					{
						led_bat_lev = target_bat_lev;
					}
					break;

				case LED_BAT_LEV_2:
					if (discharge_time_cnt >= BAT_SINGLE_LIGHT_2LED_TIME)
					{
						led_bat_lev = target_bat_lev;
					}
					break;

				case LED_BAT_LEV_1:
					if (discharge_time_cnt >= BAT_SINGLE_LIGHT_1LED_TIME)
					{
						led_bat_lev = target_bat_lev;
					}
					break;

				case LED_BAT_LEV_ALERT:
					led_bat_lev = target_bat_lev;
					led_bat_lev_sta = LED_BAT_LEV_STA_ALERT;
					LED_100_PERCENT_OFF();
					LED_75_PERCENT_OFF();
					LED_50_PERCENT_OFF();
					LED_25_PERCENT_OFF();
					return;
					break;

				default:
					break;
				}
			}
			else if (led_ctl.status == LED_STATUS_WHITE_YELLOW)
			{
				// 双色灯
				switch (target_bat_lev)
				{
				case LED_BAT_LEV_3:
					if (discharge_time_cnt >= BAT_WY_3LED_TIME)
					{
						led_bat_lev = target_bat_lev;
					}
					break;

				case LED_BAT_LEV_2:
					if (discharge_time_cnt >= BAT_WY_2LED_TIME)
					{
						led_bat_lev = target_bat_lev;
					}
					break;

				case LED_BAT_LEV_1:
					if (discharge_time_cnt >= BAT_WY_1LED_TIME)
					{
						led_bat_lev = target_bat_lev;
					}
					break;

				case LED_BAT_LEV_ALERT:
					led_bat_lev = target_bat_lev;
					led_bat_lev_sta = LED_BAT_LEV_STA_ALERT;
					LED_100_PERCENT_OFF();
					LED_75_PERCENT_OFF();
					LED_50_PERCENT_OFF();
					LED_25_PERCENT_OFF();
					return;
					break;

				default:
					break;
				}
			}
		}
		else
		{
			/*
				led_bat_lev > target_bat_lev，
				并且 led_bat_lev 与 target_bat_lev 之间的差值大于1，需要跳级

				跳级之后，如果放电时间对不上，需要加上补偿
				补偿之后，放电的时间会大于当前挡位对应的放电时间
			*/
			jump_down_cnt_ms++;
			if (jump_down_cnt_ms >= LED_BAT_LEV_JUMP_DOWN_DEBOUNCE_MS)
			{
				jump_down_cnt_ms = 0;

#if USER_DEBUG_ENABLE
				printf("jump down\n");
#endif

				if (led_bat_lev > LED_BAT_LEV_ALERT)
				{
					led_bat_lev--;
				}

				if (led_ctl.status == LED_STATUS_YELLOW ||
					led_ctl.status == LED_STATUS_WHITE ||
					led_ctl.status == LED_STATUS_RED_BLUE_FLASH)
				{
					// 单色灯
					switch (led_bat_lev)
					{
					case LED_BAT_LEV_3:
						if (discharge_time_cnt < BAT_SINGLE_LIGHT_3LED_TIME)
						{
							// 放电时间小于该挡位对应的时间，进行补偿
							discharge_time_cnt =
								((BAT_SINGLE_LIGHT_3LED_TIME +
								  BAT_SINGLE_LIGHT_2LED_TIME) /
								 2);
						}
						break;

					case LED_BAT_LEV_2:
						if (discharge_time_cnt < BAT_SINGLE_LIGHT_2LED_TIME)
						{
							// 放电时间小于该挡位对应的时间，进行补偿
							discharge_time_cnt =
								((BAT_SINGLE_LIGHT_2LED_TIME +
								  BAT_SINGLE_LIGHT_1LED_TIME) /
								 2);
						}
						break;

					case LED_BAT_LEV_1:
						if (discharge_time_cnt < BAT_SINGLE_LIGHT_1LED_TIME)
						{
							// 放电时间小于该挡位对应的时间，进行补偿
							discharge_time_cnt =
								((BAT_SINGLE_LIGHT_1LED_TIME +
								  BAT_SINGLE_LIGHT_LOW_WARN_TIME) /
								 2);
						}
						break;

					default:
						break;
					}
				}
				else if (led_ctl.status == LED_STATUS_WHITE_YELLOW)
				{
					// 双色灯
					switch (led_bat_lev)
					{
					case LED_BAT_LEV_3:
						if (discharge_time_cnt < BAT_WY_3LED_TIME)
						{
							// 放电时间小于该挡位对应的时间，进行补偿
							discharge_time_cnt =
								((BAT_WY_3LED_TIME +
								  BAT_WY_2LED_TIME) /
								 2);
						}
						break;

					case LED_BAT_LEV_2:
						if (discharge_time_cnt < BAT_WY_2LED_TIME)
						{
							// 放电时间小于该挡位对应的时间，进行补偿
							discharge_time_cnt =
								((BAT_WY_2LED_TIME +
								  BAT_WY_1LED_TIME) /
								 2);
						}
						break;

					case LED_BAT_LEV_1:
						if (discharge_time_cnt < BAT_WY_1LED_TIME)
						{
							// 放电时间小于该挡位对应的时间，进行补偿
							discharge_time_cnt =
								((BAT_WY_1LED_TIME +
								  BAT_WY_LOW_WARN_TIME) /
								 2);
						}
						break;

					default:
						break;
					}
				}
			}
		}
	}

#if USER_DEBUG_ENABLE

	{
		static u16 cnt = 0;
		cnt++;
		if (cnt >= 1000)
		{
			cnt = 0;
			printf("led_bat_lev == %u\n", (u16)led_bat_lev);
		}
	}
#endif

	// 更新显示到新的档位
	switch (led_bat_lev)
	{
	case LED_BAT_LEV_OFF:
	case LED_BAT_LEV_ALERT:
		// 低电量提示，先关闭所有指示灯，由后续低电量提示的处理函数来控制动画
		led_bat_lev_sta = LED_BAT_LEV_STA_ALERT;
		LED_100_PERCENT_OFF();
		LED_75_PERCENT_OFF();
		LED_50_PERCENT_OFF();
		LED_25_PERCENT_OFF();
		break;
	case LED_BAT_LEV_1:
		LED_100_PERCENT_OFF();
		LED_75_PERCENT_OFF();
		LED_50_PERCENT_OFF();
		LED_25_PERCENT_ON();
		break;
	case LED_BAT_LEV_2:
		LED_100_PERCENT_OFF();
		LED_75_PERCENT_OFF();
		LED_50_PERCENT_ON();
		LED_25_PERCENT_ON();
		break;
	case LED_BAT_LEV_3:
		LED_100_PERCENT_OFF();
		LED_75_PERCENT_ON();
		LED_50_PERCENT_ON();
		LED_25_PERCENT_ON();
		break;
	default:
		LED_100_PERCENT_ON();
		LED_75_PERCENT_ON();
		LED_50_PERCENT_ON();
		LED_25_PERCENT_ON();
		break;
	}
}

static void led_bat_lev_sta_alert_handle(void)
{
	static volatile u16 cnt = 0; // 控制闪烁的时间间隔

	LED_100_PERCENT_OFF();
	LED_75_PERCENT_OFF();
	LED_50_PERCENT_OFF();

	cnt++;
	if (cnt >= 500)
	{
		cnt = 0;
		LED_25_PERCENT_TOGGLE();
	}
}

static void led_bat_lev_sta_idle_handle(void)
{
	LED_100_PERCENT_OFF();
	LED_75_PERCENT_OFF();
	LED_50_PERCENT_OFF();
	LED_25_PERCENT_OFF();
}

static void led_bat_lev_sta_charge_begin_handle(void)
{
	LED_100_PERCENT_OFF();
	LED_75_PERCENT_OFF();
	LED_50_PERCENT_OFF();
	LED_25_PERCENT_OFF();
	led_charge_anim_phase = 0; // 充电动画的阶段，初始化为0
	led_charge_anim_cnt = 0;   // 清空充电动画的计数值
	led_bat_lev_sta = LED_BAT_LEV_STA_CHARGE_BEGIN_ANIM;
}

static void led_bat_lev_sta_charge_begin_anim_handle(void)
{
	led_charge_anim_cnt++;
	if (led_charge_anim_cnt >= 500)
	{
		led_charge_anim_cnt = 0;
		if (led_charge_anim_phase == 0 &&
			LED_BAT_LEV_1 <= led_bat_lev)
		{
			LED_25_PERCENT_ON();
			led_charge_anim_phase = 1;
		}
		else if (led_charge_anim_phase == 1 &&
				 LED_BAT_LEV_2 <= led_bat_lev)
		{
			LED_50_PERCENT_ON();
			led_charge_anim_phase = 2;
		}
		else if (led_charge_anim_phase == 2 &&
				 LED_BAT_LEV_3 <= led_bat_lev)
		{
			LED_75_PERCENT_ON();
			led_charge_anim_phase = 3;
		}
		else
		{
			led_charge_anim_cnt = 0;

			// 充电开始动画结束，转到充电动画状态
			led_bat_lev_sta = LED_BAT_LEV_STA_CHARGING;
		}
	}
}

static void led_bat_lev_sta_charging_handle(void)
{
	static volatile u32 led_quick_fill_cnt = 0; // 充满电时，快速填充的计数值

	led_charge_anim_cnt++;
	// 正在充电，让当前电量对应的指示灯闪烁（只闪烁一个灯）
	// led_charge_anim_cnt == 0，说明刚进入这个阶段，也需要对应的指示灯状态翻转一次
	if (led_charge_anim_cnt >= 500 || led_charge_anim_cnt == 0)
	{
		led_charge_anim_cnt = 0;

		// 根据 LED_BAT_LEVEL_STA_CHARGE_BEGIN_ANIM 期间更新的值，让对应的指示灯闪烁
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

	if ((is_in_charging_by_charger || // 通过充电器充电，或者通过太阳能充电
		 is_in_charging_by_solar_panel) &&
		is_charging_ic_stop) // 充电IC报告停止，认为充满电，进入快速填充显示状态
	{
		// 如果已经是满电显示，直接标记为充满
		if (led_bat_lev >= LED_BAT_LEV_4)
		{
			led_bat_lev_sta = LED_BAT_LEV_STA_CHARGE_END;
		}
		else
		{
			// 如果已经满电，但是当前没有显示4个指示灯
			// 快速填充：以较短时间逐档上升显示（由 led_quick_fill_cnt 驱动）
			if (led_quick_fill_cnt < ((u32)-1))
			{
				led_quick_fill_cnt++;
			}

			if (led_quick_fill_cnt >= LED_CHARGE_QUICK_FILL_INTERVAL_MS)
			{
				led_quick_fill_cnt = 0;

				/*
					REVIEW, 如果 led_bat_lev 和 led_charge_anim_phase
					的数值并不对应，这里需要重新映射，建立对应关系
				*/
				led_bat_lev++;
				led_charge_anim_phase++;
			}
		}
	}
	else
	{
		led_quick_fill_cnt = 0;
	}

	if (charge_time_cnt >= BAT_CHARGE_3LED_TIME &&
		(led_charge_anim_phase == 2 ||
		 led_charge_anim_phase == 3))
	{
		led_charge_anim_phase = 3;
		LED_75_PERCENT_ON(); //

		led_bat_lev = LED_BAT_LEV_3;
	}
	else if (charge_time_cnt >= BAT_CHARGE_2LED_TIME &&
			 (led_charge_anim_phase == 1 ||
			  led_charge_anim_phase == 2))
	{
		led_charge_anim_phase = 2;
		LED_50_PERCENT_ON(); //

		led_bat_lev = LED_BAT_LEV_2;
	}
	else if (charge_time_cnt >= BAT_CHARGE_1LED_TIME &&
			 (led_charge_anim_phase == 0 ||
			  led_charge_anim_phase == 1))
	{
		led_charge_anim_phase = 1;
		LED_25_PERCENT_ON(); //

		led_bat_lev = LED_BAT_LEV_1;
	}
}

static void led_bat_lev_sta_charge_end_handle(void)
{
	// 充电结束，显示满电状态
	LED_100_PERCENT_ON();
	LED_75_PERCENT_ON();
	LED_50_PERCENT_ON();
	LED_25_PERCENT_ON();
}

void led_bat_lev_dislplay_1ms_isr(void)
{
	switch (led_bat_lev_sta)
	{
	case LED_BAT_LEV_STA_IDLE:
		led_bat_lev_sta_idle_handle();
		break;
	case LED_BAT_LEV_STA_CHARGE_BEGIN:
		led_bat_lev_sta_charge_begin_handle();
		break;

	case LED_BAT_LEV_STA_CHARGE_BEGIN_ANIM:
		led_bat_lev_sta_charge_begin_anim_handle();
		break;

	case LED_BAT_LEV_STA_CHARGING:
		led_bat_lev_sta_charging_handle();
		break;

	case LED_BAT_LEV_STA_CHARGE_END:
		led_bat_lev_sta_charge_end_handle();
		break;

	case LED_BAT_LEV_STA_ALERT:
		led_bat_lev_sta_alert_handle();
		break;

	case LED_BAT_LEV_STA_DISCHARGE:
		led_bat_lev_sta_discharge_handle();
		break;
	}
}
