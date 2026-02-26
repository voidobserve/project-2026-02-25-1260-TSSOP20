#ifndef __AD_KEY_H__
#define __AD_KEY_H__

#include "include.h"
#include "key_driver.h"

/*
    ad按键上拉电阻一侧的电压，转换成对应的ad值：
    由于ad key上拉连接的是BAT，ad参考电压选用BAT，
    采集到的ad值就是 4095
*/
#define AD_KEY_PULL_UP_VOLTAGE_VAL ((u16)4095)

#define AD_KEY_PULL_UP_VAL ((u32)10 * 1000) // 按键上拉电阻阻值，单位：R

// ad按键下拉电阻阻值，按从小到大排列（注意要修饰成u32类型，否则下面的计算会溢出），单位：R
#define AD_KEY_PULL_DOWN_VAL_1 ((u32)0)         //
#define AD_KEY_PULL_DOWN_VAL_2 ((u32)2200)      //
#define AD_KEY_PULL_DOWN_VAL_3 ((u32)7500)      //
#define AD_KEY_PULL_DOWN_VAL_4 ((u32)47 * 1000) //

// #define AD_KEY_1_PULL_PD ((u32)2200)      // 按键 1 对应的下拉电阻阻值，单位：R
// #define AD_KEY_2_PULL_PD ((u32)7500)      // 按键 2 对应的下拉电阻阻值，单位：R
// #define AD_KEY_3_PULL_PD ((u32)47 * 1000) // 按键 3 对应的下拉电阻阻值，单位：R
// #define AD_KEY_4_PULL_PD ((u32)0)         // 按键 4 对应的下拉电阻阻值，单位：R

// 按键按下时，对应的ad值：
#define AD_KEY_NONE_VAL ((u16)4095) // 没有按键按下，对应的ad值
#define AD_KEY_1_PRESS_VAL ((u16)(AD_KEY_PULL_UP_VOLTAGE_VAL * AD_KEY_PULL_DOWN_VAL_1 / (AD_KEY_PULL_UP_VAL + AD_KEY_PULL_DOWN_VAL_1)))
#define AD_KEY_2_PRESS_VAL ((u16)(AD_KEY_PULL_UP_VOLTAGE_VAL * AD_KEY_PULL_DOWN_VAL_2 / (AD_KEY_PULL_UP_VAL + AD_KEY_PULL_DOWN_VAL_2)))
#define AD_KEY_3_PRESS_VAL ((u16)(AD_KEY_PULL_UP_VOLTAGE_VAL * AD_KEY_PULL_DOWN_VAL_3 / (AD_KEY_PULL_UP_VAL + AD_KEY_PULL_DOWN_VAL_3)))
#define AD_KEY_4_PRESS_VAL ((u16)(AD_KEY_PULL_UP_VOLTAGE_VAL * AD_KEY_PULL_DOWN_VAL_4 / (AD_KEY_PULL_UP_VAL + AD_KEY_PULL_DOWN_VAL_4)))

/*
    按键按下后，ad检测不一定能得到对应的ad值，这里再根据各个ad按键的ad值来划分阈值,
    小于该值，说明对应按键按下
*/ 
#define AD_KEY_INDEX_1_VAL ((u16)(((u32)AD_KEY_1_PRESS_VAL + AD_KEY_2_PRESS_VAL) / 2))
#define AD_KEY_INDEX_2_VAL ((u16)(((u32)AD_KEY_2_PRESS_VAL + AD_KEY_3_PRESS_VAL) / 2))
#define AD_KEY_INDEX_3_VAL ((u16)(((u32)AD_KEY_3_PRESS_VAL + AD_KEY_4_PRESS_VAL) / 2))
#define AD_KEY_INDEX_4_VAL ((u16)(((u32)AD_KEY_4_PRESS_VAL + AD_KEY_NONE_VAL) / 2)) 

#define AD_KEY_EFFECT_EVENT_NUMS (4) // 单个ad按键的有效按键事件个数（click、long、hold、loose）

#define AD_KEY_SCAN_CIRCLE_TIMES (10)
#define AD_KEY_FILTER_TIMES (3) // 触摸按键消抖次数 
#define AD_KEY_LONG_PRESS_TIME_THRESHOLD_MS (500) // 长按时间阈值（单位：ms）
#define AD_KEY_HOLD_PRESS_TIME_THRESHOLD_MS (150) // 长按持续(不松手)的时间阈值(单位：ms)，每隔 xx 时间认为有一次长按持续事件

// 定义按键的索引
enum
{
    AD_KEY_INDEX_NONE = 0,
    AD_KEY_INDEX_1, 
    AD_KEY_INDEX_2,
    AD_KEY_INDEX_3,
    AD_KEY_INDEX_4,
};

// 定义ad按键的按键事件
enum AD_KEY_EVENT
{
    AD_KEY_EVENT_NONE,
    AD_KEY_EVENT_ID_1_CLICK, 
    AD_KEY_EVENT_ID_1_LONG,
    AD_KEY_EVENT_ID_1_HOLD,
    AD_KEY_EVENT_ID_1_LOOSE,

    AD_KEY_EVENT_ID_2_CLICK, 
    AD_KEY_EVENT_ID_2_LONG,
    AD_KEY_EVENT_ID_2_HOLD,
    AD_KEY_EVENT_ID_2_LOOSE,

    AD_KEY_EVENT_ID_3_CLICK, 
    AD_KEY_EVENT_ID_3_LONG,
    AD_KEY_EVENT_ID_3_HOLD,
    AD_KEY_EVENT_ID_3_LOOSE,

    AD_KEY_EVENT_ID_4_CLICK, 
    AD_KEY_EVENT_ID_4_LONG,
    AD_KEY_EVENT_ID_4_HOLD,
    AD_KEY_EVENT_ID_4_LOOSE,

    AD_KEY_EVENT_ID_5_CLICK, 
    AD_KEY_EVENT_ID_5_LONG,
    AD_KEY_EVENT_ID_5_HOLD,
    AD_KEY_EVENT_ID_5_LOOSE,
};
 

extern volatile struct key_driver_para ad_key_para; 
 
// void adc_update_ad_key_val(u16 adc_val);
// u16 adc_get_ad_key_val(void);

void ad_key_handle(void);

#endif