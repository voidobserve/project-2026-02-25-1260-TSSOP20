#include "led_bat_lev.h"
#include "led.h"

#include "bat_scan.h"
#include "charge_det.h"
#include "bluetooth_ic_handle.h"
#include "battery_monitor.h"

// 电池电量指示灯挡位
volatile led_bat_lev_t led_bat_lev = LED_BAT_LEV_OFF;
// 电池电量指示灯状态
volatile led_bat_lev_sta_t led_bat_lev_sta = LED_BAT_LEV_STA_IDLE;

// const u16 bat_lev_discharge_time_map_in_single_light[] = {
// 	BAT_SINGLE_LIGHT_1LED_TIME,
// 	BAT_SINGLE_LIGHT_2LED_TIME,
// 	BAT_SINGLE_LIGHT_3LED_TIME,
// };

// REVIEW 在第一次上电、低功耗唤醒后使用
void led_bat_lev_init(u8 dest_bat_lev)
{
}

void led_bat_lev_init_by_vol(u16 voltage_mv)
{
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
		led_bat_lev_sta = LED_BAT_LEV_STA_DISCHARGE;
	}
}

static void led_bat_lev_sta_charge_begin_anim_handle(void)
{
}

static void led_bat_lev_sta_charging_handle(void)
{
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
			// TODO 如果此时显示的挡位还不是1个指示灯，需要快速递减
			// led_bat_lev_sta = LED_BAT_LEV_STA_ALERT; // 转到低电量提示状态
			// return; // 提前返回，不执行后续操作

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
			// 特殊情况
		}
	}
	else if (led_ctl.status == LED_STATUS_WHITE_YELLOW)
	{
		// 黄白灯都亮
		if (discharge_time_cnt >= BAT_WY_LOW_WARN_TIME &&
			avg_voltage_mv <
				(BAT_WY_LOW_WARN_VOLTAGE -
				 BAT_WY_DEAD_ZONE_VOLTAGE))
		{
			is_in_low_bat_alert = 1;
			target_bat_lev = LED_BAT_LEV_STA_ALERT;
		}
		else if (discharge_time_cnt >= BAT_WY_1LED_TIME &&
				 avg_voltage_mv <
					 (BAT_WY_2LED_VOLTAGE -
					  BAT_WY_DEAD_ZONE_VOLTAGE))
		{
			target_bat_lev = LED_BAT_LEV_1;
		}
		else if (discharge_time_cnt >= BAT_WY_2LED_TIME &&
				 avg_voltage_mv <
					 (BAT_WY_3LED_VOLTAGE -
					  BAT_WY_DEAD_ZONE_VOLTAGE))
		{
			target_bat_lev = LED_BAT_LEV_2;
		}
		else if (discharge_time_cnt >= BAT_WY_3LED_TIME &&
				 avg_voltage_mv <
					 (BAT_WY_4LED_VOLTAGE -
					  BAT_WY_DEAD_ZONE_VOLTAGE))
		{
			target_bat_lev = LED_BAT_LEV_3;
		}
		else
		{
			target_bat_lev = LED_BAT_LEV_4;
		}
	}

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
			}
		}
		else
		{
			/*
				跳级
				跳级之后，如果放电时间对不上，需要加上补偿
				补偿之后，放电的时间会大于当前挡位对应的放电时间
			*/
			jump_down_cnt_ms++;
			if (jump_down_cnt_ms >= LED_BAT_LEV_JUMP_DOWN_DEBOUNCE_MS)
			{
				jump_down_cnt_ms = 0;

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

						break;

					default:
						break;
					}
				}
				else if (led_ctl.status == LED_STATUS_WHITE_YELLOW)
				{
					// 双色灯
				}
			}
		}
	}

	// 更新显示到新的档位
	switch (led_bat_lev)
	{
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

void led_bat_lev_dislplay_1ms_isr(void)
{
	switch (led_bat_lev_sta)
	{
	case LED_BAT_LEV_STA_IDLE:

		break;
	case LED_BAT_LEV_STA_CHARGE_BEGIN:

		break;

	case LED_BAT_LEV_STA_CHARGE_BEGIN_ANIM:
		break;

	case LED_BAT_LEV_STA_CHARGING:
		led_bat_lev_sta_charging_handle();
		break;

	case LED_BAT_LEV_STA_CHARGE_END:
		break;

	case LED_BAT_LEV_STA_ALERT:
		break;

	case LED_BAT_LEV_STA_DISCHARGE:
		led_bat_lev_sta_discharge_handle();
		break;
	}
}

#if 0
void led_bat_lev_handle_1ms_isr(void)
{
	if (led_bat_level_sta == LED_BAT_LEVEL_STA_IDLE)
	{
		LED_100_PERCENT_OFF();
		LED_75_PERCENT_OFF();
		LED_50_PERCENT_OFF();
		LED_25_PERCENT_OFF();
	}
	else if (led_bat_level_sta == LED_BAT_LEVEL_STA_CHARGE_BEGIN)
	{
		LED_100_PERCENT_OFF();
		LED_75_PERCENT_OFF();
		LED_50_PERCENT_OFF();
		LED_25_PERCENT_OFF();
		led_charge_anim_phase = 0;
		led_charge_anim_cnt = 0;
		led_quick_fill_cnt = 0;
		led_bat_level_sta = LED_BAT_LEVEL_STA_CHARGE_BEGIN_ANIM;
	}
	else if (led_bat_level_sta == LED_BAT_LEVEL_STA_CHARGE_BEGIN_ANIM)
	{
		led_charge_anim_cnt++;
		if (led_charge_anim_cnt >= 500)
		{
			led_charge_anim_cnt = 0;

			if (led_charge_anim_phase == 0 &&
				LED_BAT_LEV_1 <= led_bat_lev_sta)
			{
				LED_25_PERCENT_ON();
				led_charge_anim_phase = 1;
			}
			else if (led_charge_anim_phase == 1 &&
					 LED_BAT_LEV_2 <= led_bat_lev_sta)
			{
				LED_50_PERCENT_ON();
				led_charge_anim_phase = 2;
			}
			else if (led_charge_anim_phase == 2 &&
					 LED_BAT_LEV_3 <= led_bat_lev_sta)
			{
				LED_75_PERCENT_ON();
				led_charge_anim_phase = 3;
			}
			else
			{
				led_charge_anim_cnt = 0;
				led_bat_level_sta = LED_BAT_LEVEL_STA_CHARGING;
			}
		}
	}
	else if (led_bat_level_sta == LED_BAT_LEVEL_STA_CHARGING)
	{
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

			if ((is_in_charging_by_charger || // 通过充电器充电，或者通过太阳能充电
				 is_in_charging_by_solar_panel) &&
				is_charging_ic_stop) // 充电IC报告停止，认为充满电，进入快速填充显示状态
			{
				// 如果已经是满电显示，直接标记为充满
				if (led_bat_lev_sta >= LED_BAT_LEV_4)
				{
					led_bat_level_sta = LED_BAT_LEVEL_STA_CHARGE_END;
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
							REVIEW, 如果 led_bat_lev_sta 和 led_charge_anim_phase
							的数值并不对应，这里需要重新映射，建立对应关系
						*/
						led_bat_lev_sta++;
						led_charge_anim_phase = led_bat_lev_sta;
					}
				}
			}
			else if (charge_time_cnt >= BAT_CHARGE_3LED_TIME &&
					 (led_charge_anim_phase == 2 ||
					  led_charge_anim_phase == 3))
			{
				led_charge_anim_phase = 3;
				LED_75_PERCENT_ON(); //

				led_bat_lev_sta = LED_BAT_LEV_3;
			}
			else if (charge_time_cnt >= BAT_CHARGE_2LED_TIME &&
					 (led_charge_anim_phase == 1 ||
					  led_charge_anim_phase == 2))
			{
				led_charge_anim_phase = 2;
				LED_50_PERCENT_ON(); //

				led_bat_lev_sta = LED_BAT_LEV_2;
			}
			else if (charge_time_cnt >= BAT_CHARGE_1LED_TIME &&
					 (led_charge_anim_phase == 0 ||
					  led_charge_anim_phase == 1))
			{
				led_charge_anim_phase = 1;
				LED_25_PERCENT_ON(); //

				led_bat_lev_sta = LED_BAT_LEV_1;
			}
		}

		led_charge_anim_cnt++;
	}
	else if (led_bat_level_sta == LED_BAT_LEVEL_STA_CHARGE_END)
	{
		// 已经充满电
		LED_100_PERCENT_ON();
		LED_75_PERCENT_ON();
		LED_50_PERCENT_ON();
		LED_25_PERCENT_ON();
		led_bat_lev_sta = LED_BAT_LEV_4;
	}
	else if (led_bat_level_sta == LED_BAT_LEVEL_STA_DISCHARGE)
	{
		// 放电中
		static volatile u8 blink_cnt = 0;		  // 闪烁计数
		static volatile u32 jump_down_cnt_ms = 0; // 跳级向下的去抖计时（ms）

		if (is_in_low_bat_alert)
		{
			// 正在低电量报警
			// 让指示灯以 1Hz 频率闪烁
			LED_100_PERCENT_OFF();
			LED_75_PERCENT_OFF();
			LED_50_PERCENT_OFF();

			blink_cnt++;
			if (blink_cnt >= 500)
			{
				blink_cnt = 0;
				LED_25_PERCENT_TOGGLE();
			}

			led_bat_lev_sta = LED_BAT_LEV_STA_ALERT;
			jump_down_cnt_ms = 0;
		}
		else
		{

			volatile u8 target_level = LED_BAT_LEV_4;
			blink_cnt = 0; // 不在低电量报警，清空低电量报警的闪烁计数

			if (led_ctl.status == LED_STATUS_YELLOW ||
				led_ctl.status == LED_STATUS_WHITE ||
				led_ctl.status == LED_STATUS_RED_BLUE_FLASH)
			{
				// 只有一种灯亮（single light）
				if (discharge_time_cnt >= BAT_SINGLE_LIGHT_LOW_WARN_TIME &&
					avg_voltage_mv <
						(BAT_SINGLE_LIGHT_LOW_WARN_VOLTAGE -
						 BAT_SINGLE_LIGHT_DEAD_ZONE_VOLTAGE))
				{
					is_in_low_bat_alert = 1;
					target_level = LED_BAT_LEV_STA_ALERT;
				}
				else if (discharge_time_cnt >= BAT_SINGLE_LIGHT_1LED_TIME &&
						 avg_voltage_mv <
							 (BAT_SINGLE_LIGHT_2LED_VOLTAGE -
							  BAT_SINGLE_LIGHT_DEAD_ZONE_VOLTAGE))
				{
					target_level = LED_BAT_LEV_1;
				}
				else if (discharge_time_cnt >= BAT_SINGLE_LIGHT_2LED_TIME &&
						 avg_voltage_mv <
							 (BAT_SINGLE_LIGHT_3LED_VOLTAGE -
							  BAT_SINGLE_LIGHT_DEAD_ZONE_VOLTAGE))
				{
					target_level = LED_BAT_LEV_2;
				}
				else if (discharge_time_cnt >= BAT_SINGLE_LIGHT_3LED_TIME &&
						 avg_voltage_mv <
							 (BAT_SINGLE_LIGHT_4LED_VOLTAGE -
							  BAT_SINGLE_LIGHT_DEAD_ZONE_VOLTAGE))
				{
					target_level = LED_BAT_LEV_3;
				}
				else
				{
					target_level = LED_BAT_LEV_4;
				}
			}
			else if (led_ctl.status == LED_STATUS_WHITE_YELLOW)
			{
				// 黄白灯都亮
				if (discharge_time_cnt >= BAT_WY_LOW_WARN_TIME &&
					avg_voltage_mv <
						(BAT_WY_LOW_WARN_VOLTAGE -
						 BAT_WY_DEAD_ZONE_VOLTAGE))
				{
					is_in_low_bat_alert = 1;
					target_level = LED_BAT_LEV_STA_ALERT;
				}
				else if (discharge_time_cnt >= BAT_WY_1LED_TIME &&
						 avg_voltage_mv <
							 (BAT_WY_2LED_VOLTAGE -
							  BAT_WY_DEAD_ZONE_VOLTAGE))
				{
					target_level = LED_BAT_LEV_1;
				}
				else if (discharge_time_cnt >= BAT_WY_2LED_TIME &&
						 avg_voltage_mv <
							 (BAT_WY_3LED_VOLTAGE -
							  BAT_WY_DEAD_ZONE_VOLTAGE))
				{
					target_level = LED_BAT_LEV_2;
				}
				else if (discharge_time_cnt >= BAT_WY_3LED_TIME &&
						 avg_voltage_mv <
							 (BAT_WY_4LED_VOLTAGE -
							  BAT_WY_DEAD_ZONE_VOLTAGE))
				{
					target_level = LED_BAT_LEV_3;
				}
				else
				{
					target_level = LED_BAT_LEV_4;
				}
			}

			// 如果目标等级高于或等于当前显示等级，立即更新并清除跳级计时
			if (target_level >= led_bat_lev_sta)
			{
				jump_down_cnt_ms = 0;
				led_bat_lev_sta = target_level;
				// 立即更新显示
				switch (led_bat_lev_sta)
				{
				case LED_BAT_LEV_STA_ALERT:
					LED_100_PERCENT_OFF();
					LED_75_PERCENT_OFF();
					LED_50_PERCENT_OFF();
					LED_25_PERCENT_ON();
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
			else if (((target_level + 1) < led_bat_lev_sta) ||
					 (target_level == LED_BAT_LEV_1))
			{
				/*
					目标等级比当前显示等级还要低2级，或者是目标等级已经是1级，
					按每次降一档的策略，每隔 LED_BAT_JUMP_DOWN_DEBOUNCE_MS 降一档
				*/
				if (jump_down_cnt_ms < ((u32)-1))
				{
					jump_down_cnt_ms++;
				}

				if (jump_down_cnt_ms >= LED_BAT_JUMP_DOWN_DEBOUNCE_MS)
				{
					jump_down_cnt_ms = 0;
					if (led_bat_lev_sta > LED_BAT_LEV_1)
					{
						led_bat_lev_sta--;
					}

					// 更新显示到新的档位
					switch (led_bat_lev_sta)
					{
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
			}
		}
	}
}
#endif