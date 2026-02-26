#ifndef __ADC_H__
#define __ADC_H__

#include "include.h"

// 定义检测adc的通道:
enum
{
    ADC_SEL_PIN_NONE = 0, 

    ADC_CHANNEL_SEL_AD_KEY, // ad按键检测
    ADC_CHANNEL_SEL_BAT_DET, // 电池电量检测
    ADC_CHANNEL_SEL_SOLAR_DET, // 太阳能充电检测
};
typedef u8 adc_channel_sel_t;

enum
{
    ADC_STATUS_IDLE = 0,
    ADC_STATUS_SEL_AD_KEY_WAITING, // 等待adc稳定
    ADC_STATUS_SEL_AD_KEY,
    ADC_STATUS_SEL_BAT_DET_WAITING,
    ADC_STATUS_SEL_BAT_DET,
    ADC_STATUS_SEL_SOLAR_DET_WAITING,
    ADC_STATUS_SEL_SOLAR_DET, 
};
 

extern volatile u8 cur_adc_status; // 状态机，表示当前adc的状态 
  

void adc_pin_init(void); // adc相关的引脚配置，调用完成后，还未能使用adc

void adc_init(void);  
void adc_scan(void);

void adc_update_val(adc_channel_sel_t adc_channel, u16 adc_val);
u16 adc_get_val(adc_channel_sel_t adc_channel);
u8 adc_get_update_flag(adc_channel_sel_t adc_channel);
void adc_clear_update_flag(adc_channel_sel_t adc_channel);


#endif