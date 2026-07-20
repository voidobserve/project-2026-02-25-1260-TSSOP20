#ifndef __LED_BAT_LEV_H__
#define __LED_BAT_LEV_H__

#include "typedef.h"
#include "user_config.h"

// TEST_ONLY
#define BAT_LED_TEST_ENABLE 0 // 电池电量指示灯测试

// =====================================================================
// =====================================================================

// 以下电池电压和电量百分比的关机由电池的充放电曲线得出

// 放电时，电池电量百分比对应的电池电压，单位：mV
#define LED_BAT_LEV_25_PERCENT_MAP_TO_VOLTAGE_DURING_DISCHARGING \
	((u16)3710)
#define LED_BAT_LEV_50_PERCENT_MAP_TO_VOLTAGE_DURING_DISCHARGING \
	((u16)3800)
#define LED_BAT_LEV_75_PERCENT_MAP_TO_VOLTAGE_DURING_DISCHARGING \
	((u16)3920)
#define LED_BAT_LEV_100_PERCENT_MAP_TO_VOLTAGE_DURING_DISCHARGING \
	((u16)4200)

// 充电时，电池电量百分比对应的电池电压，单位：mV
#define LED_BAT_LEV_25_PERCENT_MAP_TO_VOLTAGE_DURING_CHARGING \
	((u16)3920)
#define LED_BAT_LEV_50_PERCENT_MAP_TO_VOLTAGE_DURING_CHARGING \
	((u16)4100)
#define LED_BAT_LEV_75_PERCENT_MAP_TO_VOLTAGE_DURING_CHARGING \
	((u16)4150)
#define LED_BAT_LEV_100_PERCENT_MAP_TO_VOLTAGE_DURING_CHARGING \
	((u16)4200)
// =====================================================================
// =====================================================================

// 黄白灯一起亮（ YELLOW + WHITE ）
#define BAT_WY_3LED_VOLTAGE ((u16)3700)
#define BAT_WY_2LED_VOLTAGE ((u16)3600)
#define BAT_WY_1LED_VOLTAGE ((u16)3500)
#define BAT_WY_LOW_WARN_VOLTAGE ((u16)3250)
// 死区电压
#define BAT_WY_DEAD_ZONE_VOLTAGE ((u16)50)

#if (!BAT_LED_TEST_ENABLE)
// 放电时，电池电量指示灯只显示 3颗 时，对应的放电时间（从满电开始计算），单位：s
#define BAT_WY_3LED_TIME ((u16)60 * 60)
// 放电时，电池电量指示灯只显示 2颗 时，对应的放电时间（从满电开始计算），单位：s
#define BAT_WY_2LED_TIME ((u16)120 * 60)
// 放电时，电池电量指示灯只显示 1颗 时，对应的放电时间（从满电开始计算），单位：s
#define BAT_WY_1LED_TIME ((u16)180 * 60)
// 放电时，电池电量指示灯进行 低电量提示 时，对应的放电时间（从满电开始计算），单位：s
#define BAT_WY_LOW_WARN_TIME ((u16)220 * 60)
#else
// 放电时，电池电量指示灯只显示 3颗 时，对应的放电时间（从满电开始计算），单位：s
#define BAT_WY_3LED_TIME ((u16)6)
// 放电时，电池电量指示灯只显示 2颗 时，对应的放电时间（从满电开始计算），单位：s
#define BAT_WY_2LED_TIME ((u16)12)
// 放电时，电池电量指示灯只显示 1颗 时，对应的放电时间（从满电开始计算），单位：s
#define BAT_WY_1LED_TIME ((u16)18)
// 放电时，电池电量指示灯进行 低电量提示 时，对应的放电时间（从满电开始计算），单位：s
#define BAT_WY_LOW_WARN_TIME ((u16)22)
#endif
// =========================================================================

// =========================================================================
// 白灯或者黄灯单独亮（ WHITE or YELLOW only ）
// 指示灯各个挡位对应的门限电压，小于该电压，说明只能亮对应挡位的指示灯
#define BAT_SINGLE_LIGHT_3LED_VOLTAGE ((u16)3800)
#define BAT_SINGLE_LIGHT_2LED_VOLTAGE ((u16)3700)
#define BAT_SINGLE_LIGHT_1LED_VOLTAGE ((u16)3600)
#define BAT_SINGLE_LIGHT_LOW_WARN_VOLTAGE ((u16)3250)
// 死区电压
#define BAT_SINGLE_LIGHT_DEAD_ZONE_VOLTAGE ((u16)50)

#if (!BAT_LED_TEST_ENABLE)
// 放电时，电池电量指示灯只显示 3颗 时，对应的放电时间（从满电开始计算），单位：s
#define BAT_SINGLE_LIGHT_3LED_TIME ((u16)90 * 60)
#define BAT_SINGLE_LIGHT_2LED_TIME ((u16)170 * 60)
#define BAT_SINGLE_LIGHT_1LED_TIME ((u16)260 * 60)
#define BAT_SINGLE_LIGHT_LOW_WARN_TIME ((u16)310 * 60)
#else
// 放电时，电池电量指示灯只显示 3颗 时，对应的放电时间（从满电开始计算），单位：s
#define BAT_SINGLE_LIGHT_3LED_TIME ((u16)9)
#define BAT_SINGLE_LIGHT_2LED_TIME ((u16)17)
#define BAT_SINGLE_LIGHT_1LED_TIME ((u16)26)
#define BAT_SINGLE_LIGHT_LOW_WARN_TIME ((u16)31)
#endif

// =====================================================================
// =====================================================================
// 根据电池的充电曲线，得到对应的电压，单位：mV
#define BAT_CHARGE_1LED_VOLTAGE ((u16)3920)
#define BAT_CHARGE_2LED_VOLTAGE ((u16)4000)
#define BAT_CHARGE_3LED_VOLTAGE ((u16)4140)

#if (!BAT_LED_TEST_ENABLE)
// 充电时，电量各个指示灯所需的充电时间，单位：s
// 备注：最后一个灯要等充电ic输出充满电的信号
#define BAT_CHARGE_1LED_TIME ((u16)90 * 60)
#define BAT_CHARGE_2LED_TIME ((u16)90 * 2 * 60)
#define BAT_CHARGE_3LED_TIME ((u16)90 * 3 * 60)
#define BAT_CHARGE_4LED_TIME ((u16)90 * 4 * 60) // 不使用这个时间，只作为计算使用
#else
// 充电时，电量各个指示灯所需的充电时间，单位：s
// 备注：最后一个灯要等充电ic输出充满电的信号
#define BAT_CHARGE_1LED_TIME ((u16)9)
#define BAT_CHARGE_2LED_TIME ((u16)9 * 2)
#define BAT_CHARGE_3LED_TIME ((u16)9 * 3)
#define BAT_CHARGE_4LED_TIME ((u16)9 * 4) // 不使用这个时间，只作为计算使用
#endif

// =====================================================================
// =====================================================================

#if (!BAT_LED_TEST_ENABLE)
// 电池指示灯跳级向下的去抖时间（ms），发生跳级时，每隔该时间降一档
// 可能需要1分钟之内就实现跳级
#define LED_BAT_LEV_JUMP_DOWN_DEBOUNCE_MS ((u32)1 * 60 * 1000)
#else
// 电池指示灯跳级向下的去抖时间（ms），发生跳级时，每隔该时间降一档
#define LED_BAT_LEV_JUMP_DOWN_DEBOUNCE_MS ((u32)5 * 1000)
#endif

// 充电完成后快速填充每档间隔（ms），短时间内逐档上升显示
#define LED_CHARGE_QUICK_FILL_INTERVAL_MS ((u32)1 * 10 * 1000)

// 电池电量指示灯对应的挡位
enum
{
	LED_BAT_LEV_OFF = 0x00, // 关灯
	LED_BAT_LEV_ALERT,
	LED_BAT_LEV_1, //
	LED_BAT_LEV_2, //
	LED_BAT_LEV_3, //
	LED_BAT_LEV_4, //
};
typedef u8 led_bat_lev_t;

enum
{
	LED_BAT_LEV_STA_IDLE = 0x00,
	LED_BAT_LEV_STA_CHARGE_BEGIN,	   // 充电开始
	LED_BAT_LEV_STA_CHARGE_BEGIN_ANIM, // 正在跑充电开始的动画
	LED_BAT_LEV_STA_CHARGING,		   // 充电中
	LED_BAT_LEV_STA_CHARGE_END,		   // 充电结束

	LED_BAT_LEV_STA_ALERT, // 低电量闪烁（低电量提示，让最后一格对应的指示灯闪烁）
	LED_BAT_LEV_STA_DISCHARGE,
};
typedef u8 led_bat_lev_sta_t;

extern volatile led_bat_lev_t led_bat_lev;
extern volatile led_bat_lev_sta_t led_bat_lev_sta;

void led_bat_lev_init_by_vol(u16 voltage_mv);

void led_bat_lev_handle(void);

void led_bat_lev_dislplay_1ms_isr(void);

#endif