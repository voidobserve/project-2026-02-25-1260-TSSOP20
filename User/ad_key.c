#include "ad_key.h"
#include "adc.h"

// 存放按键对应的ad值:
static const u16 ad_key_scan_table[][2] = {
    // [][0]按键对应的索引,在判断按键键值时使用   [][1]按键对应的ad值阈值

    {AD_KEY_INDEX_1, AD_KEY_INDEX_1_VAL}, //
    {AD_KEY_INDEX_2, AD_KEY_INDEX_2_VAL}, //
    {AD_KEY_INDEX_3, AD_KEY_INDEX_3_VAL}, //
    {AD_KEY_INDEX_4, AD_KEY_INDEX_4_VAL}, //
};

// 将按键id和按键事件绑定起来，在 xx 函数中，通过查表的方式得到按键事件
static const u8 ad_key_event_table[][AD_KEY_EFFECT_EVENT_NUMS + 1] = {
    {AD_KEY_INDEX_1, AD_KEY_EVENT_ID_1_CLICK, AD_KEY_EVENT_ID_1_LONG, AD_KEY_EVENT_ID_1_HOLD, AD_KEY_EVENT_ID_1_LOOSE}, //
    {AD_KEY_INDEX_2, AD_KEY_EVENT_ID_2_CLICK, AD_KEY_EVENT_ID_2_LONG, AD_KEY_EVENT_ID_2_HOLD, AD_KEY_EVENT_ID_2_LOOSE}, //
    {AD_KEY_INDEX_3, AD_KEY_EVENT_ID_3_CLICK, AD_KEY_EVENT_ID_3_LONG, AD_KEY_EVENT_ID_3_HOLD, AD_KEY_EVENT_ID_3_LOOSE}, //
    {AD_KEY_INDEX_4, AD_KEY_EVENT_ID_4_CLICK, AD_KEY_EVENT_ID_4_LONG, AD_KEY_EVENT_ID_4_HOLD, AD_KEY_EVENT_ID_4_LOOSE}, //
};

extern u8 ad_key_get_key_val(void);
volatile struct key_driver_para ad_key_para = {
    // 编译器不支持指定成员赋值的写法，会报错
    AD_KEY_SCAN_CIRCLE_TIMES, // const u8 scan_times; // 按键扫描频率, 单位ms
    0,                        // volatile u8 cur_scan_times; // 按键扫描频率, 单位ms，由1ms的定时器中断内累加，在key_driver_scan()中清零
    0,                        // volatile u8 last_key; // 存放上一次调用get_value()之后得到的按键值

    //== 用于消抖类参数 ==//
    0, // volatile u8 filter_value; // 用于按键消抖，存放消抖期间得到的键值
    0, // volatile u8 filter_cnt;   // 用于按键消抖时的累加值
    3, // const u8 filter_time;     // 当filter_cnt累加到base_cnt值时, 消抖有效

    //== 用于判定长按和HOLD事件参数
    AD_KEY_LONG_PRESS_TIME_THRESHOLD_MS / AD_KEY_SCAN_CIRCLE_TIMES,                                         // const u8 long_time; // 按键判定长按数量
    (AD_KEY_LONG_PRESS_TIME_THRESHOLD_MS + AD_KEY_HOLD_PRESS_TIME_THRESHOLD_MS) / AD_KEY_SCAN_CIRCLE_TIMES, // const u8 hold_time; // 按键判定HOLD数量
    0,                                                                                                      // volatile u8 press_cnt; // 与long_time和hold_time对比, 判断long_event和hold_event

    //== 用于判定连击事件参数
    0,                  // volatile u8 click_cnt; // 按键按下次数
    0,                  // volatile u8 click_delay_cnt; // 按键被抬起后等待连击事件延时计数
    0,                  // const u8 click_delay_time; // 按键被抬起后等待连击事件延时数量
    0,                  // volatile u8 notify_value;    // 在延时的待发送按键值
    KEY_TYPE_AD,        // const u8 key_type;
    ad_key_get_key_val, // u8 (*get_value)(void); // 用户自定义的获取键值的函数

    // == 存放得到的按键键值和按键事件
    AD_KEY_INDEX_NONE, // volatile u8 latest_key_val;
    KEY_EVENT_NONE,    // volatile u8 latest_key_event;
};

// 将采集到的ad值转换成自定义的键值

static u16 ad_key_val_to_ad_key_index(const u16 ad_key_val)
{
    u8 i = 0;
    u16 ad_key_index = NO_KEY;

    // ARRAY_SIZE(ad_key_scan_table) 这里是求出数组中存放的按键个数
    for (i = 0; i < ARRAY_SIZE(ad_key_scan_table); i++)
    {
        if (ad_key_val < ad_key_scan_table[i][1])
        {
            ad_key_index = ad_key_scan_table[i][0];
            break;
        }
    }

    return ad_key_index;
}

//
/**
 * @brief 将按键值和key_driver_scan得到的按键事件转换成ad按键的事件
 *
 * @param key_val ad按键键值
 * @param key_event 在key_driver_scan得到的按键事件 KEY_EVENT
 * @return u8 在ad_key_event_table中找到的对应的按键事件，如果没有则返回 AD_KEY_EVENT_NONE
 */
static u8 ad_key_get_event(const u8 key_val, const u8 key_event)
{
    u8 ret_key_event = AD_KEY_EVENT_NONE;
    u8 key_event_index = 0;
    u8 i = 0;

    // 将 key_driver_scan得到的按键事件 KEY_EVENT 转换为 ad_key_event_table 中的索引
    if (key_event == KEY_EVENT_CLICK)
    {
        key_event_index = 1;
    }
    else if (key_event == KEY_EVENT_LONG)
    {
        key_event_index = 2;
    }
    else if (key_event == KEY_EVENT_HOLD)
    {
        key_event_index = 3;
    }
    else if (key_event == KEY_EVENT_UP)
    {
        key_event_index = 4;
    }
    else
    {
        return (u8)AD_KEY_EVENT_NONE;
    }

    for (; i < ARRAY_SIZE(ad_key_event_table); i++)
    {
        if (key_val == ad_key_event_table[i][0])
        {
            ret_key_event = ad_key_event_table[i][key_event_index];
            break;
        }
    }

    return ret_key_event;
}

// void adc_update_ad_key_val(u16 adc_val)
// {
//     adc_val_of_adkey = adc_val;
// }

// u16 adc_get_ad_key_val(void)
// {
//     return adc_val_of_adkey;
// }

// 获取按键键值，由key_driver_scan()函数调用
u8 ad_key_get_key_val(void)
{
    static volatile u16 val = AD_KEY_NONE_VAL; // 单次按键标志
    u8 ret = NO_KEY;

    // 有数据更新，才获取数据
    if (adc_get_update_flag(ADC_CHANNEL_SEL_AD_KEY))
    {
        adc_clear_update_flag(ADC_CHANNEL_SEL_AD_KEY);
        val = adc_get_val(ADC_CHANNEL_SEL_AD_KEY);   
        
        // printf("ad key val == %u\n", val);
    }

    ret = ad_key_val_to_ad_key_index(val); // 将采集到的ad值转换成自定义的键值
    return ret;
}

void ad_key_handle(void)
{
    u8 ad_key_event = AD_KEY_EVENT_NONE;

    if (ad_key_para.latest_key_val == AD_KEY_INDEX_NONE)
    {
        return;
    }

    ad_key_event = ad_key_get_event(ad_key_para.latest_key_val, ad_key_para.latest_key_event);
    ad_key_para.latest_key_val = AD_KEY_INDEX_NONE;
    ad_key_para.latest_key_event = KEY_EVENT_NONE;

    switch (ad_key_event)
    {
        // ================================================================
        // key 1，对应的丝印是 上一曲
    case AD_KEY_EVENT_ID_1_CLICK:
        printf("key 1 click\n");
        break;

    case AD_KEY_EVENT_ID_1_LONG:
        printf("key 1 long\n");
        break;

    case AD_KEY_EVENT_ID_1_HOLD:
        printf("key 1 hold\n");
        break;

    case AD_KEY_EVENT_ID_1_LOOSE:
        printf("key 1 loose\n");
        break;

        // ================================================================
        // key 2，对应的丝印是 灯开关
    case AD_KEY_EVENT_ID_2_CLICK:
        printf("key 2 click\n");
        break;

    case AD_KEY_EVENT_ID_2_LONG:
        printf("key 2 long\n");
        break;

    case AD_KEY_EVENT_ID_2_HOLD:
        printf("key 2 hold\n");
        break;

    case AD_KEY_EVENT_ID_2_LOOSE:
        printf("key 2 loose\n");
        break;

        // ================================================================
        // key 3，对应的丝印是 下一曲 

    case AD_KEY_EVENT_ID_3_CLICK:
        printf("key 3 click\n");
        break;

    case AD_KEY_EVENT_ID_3_LONG:
        printf("key 3 long\n");
        break;

    case AD_KEY_EVENT_ID_3_HOLD:
        printf("key 3 hold\n");
        break;

    case AD_KEY_EVENT_ID_3_LOOSE:
        printf("key 3 loose\n");
        break;

        // ================================================================
        // key 4，对应的丝印是 总开关 
    case AD_KEY_EVENT_ID_4_CLICK:
        printf("key 4 click\n");
        break;

    case AD_KEY_EVENT_ID_4_LONG:
        printf("key 4 long\n");
        break;

    case AD_KEY_EVENT_ID_4_HOLD:
        printf("key 4 hold\n");
        break;

    case AD_KEY_EVENT_ID_4_LOOSE:
        printf("key 4 loose\n");
        break;

    default:
        break;
    }
}