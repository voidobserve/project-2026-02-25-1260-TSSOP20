#include "adc.h"
#include "my_config.h"

volatile u8 cur_adc_status = ADC_STATUS_IDLE;

// ad按键：
static volatile u16 adc_adkey_val = 0; // 存放采集到的ad值
static volatile bit flag_is_adkey_val_update = 0;
// ad电池：
static volatile u16 adc_bat_det_val = 0;
static volatile bit flag_is_adc_bat_det_val_update = 0;
// ad太阳能：
static volatile u16 adc_solar_det_val = 0;
static volatile bit flag_is_adc_solar_det_val_update = 0;

// adc相关的引脚配置
void adc_pin_init(void)
{
    // P24 检测ADKEY
    P2_MD1 |= GPIO_P24_MODE_SEL(0x3);

    // P14 检测太阳能一侧的电压
    P1_MD1 |= GPIO_P14_MODE_SEL(0x3);
}

void adc_init(void)
{
    // ADC配置
    // ADC_ACON1 &= ~(ADC_VREF_SEL(0x07) |  // 清空参考电压选择位
    //                ADC_EXREF_SEL(0x01) | // 关闭外部参考电压
    //                ADC_INREF_SEL(0x01)); // 关闭内部参考电压
    // ADC_ACON1 |= ADC_VREF_SEL(0x06) |    // 选择 VCC 作为参考电压
    //              ADC_TEN_SEL(0x03);      // 关闭测试信号

    ADC_ACON0 = ADC_CMP_EN(0x1) |  // 打开ADC中的CMP使能信号
                ADC_BIAS_EN(0x1) | // 打开ADC偏置电流能使信号
                ADC_BIAS_SEL(0x1); // 打开 ADC偏置电流

    __EnableIRQ(ADC_IRQn); // 使能ADC中断
    IE_EA = 1;             // 使能总中断

    ADC_CFG1 |= (0x0F << 3) |       // ADC时钟分频为16分频，为系统时钟/16（把adc时钟设置为最慢，）
                (0x01 << 0);        // ADC0 通道中断使能
    ADC_CFG0 |= ADC_CHAN0_EN(0x1) | // 使能 通道0
                ADC_EN(0x1);        // 使能 adc

    delay_ms(1); // 等待ADC模块配置稳定，需要等待20us以上
}

/**
 * @brief 更新通道对应的ad值（由ad中断更新）
 *
 * @param adc_channel
 * @param adc_val
 */
void adc_update_val(adc_channel_sel_t adc_channel, u16 adc_val)
{
    switch (adc_channel)
    {
    case ADC_CHANNEL_SEL_AD_KEY:
        adc_adkey_val = adc_val;
        flag_is_adkey_val_update = 1;
        break;
    case ADC_CHANNEL_SEL_BAT_DET:
        adc_bat_det_val = adc_val;
        flag_is_adc_bat_det_val_update = 1;
        break;
    case ADC_CHANNEL_SEL_SOLAR_DET:
        adc_solar_det_val = adc_val;
        flag_is_adc_solar_det_val_update = 1;
        break;
    }
}

u16 adc_get_val(adc_channel_sel_t adc_channel)
{
    u16 ret = 0;
    switch (adc_channel)
    {
    case ADC_CHANNEL_SEL_AD_KEY:
        ret = adc_adkey_val;
        break;
    case ADC_CHANNEL_SEL_BAT_DET:
        ret = adc_bat_det_val;
        break;
    case ADC_CHANNEL_SEL_SOLAR_DET:
        ret = adc_solar_det_val;
        break;

    default:
        break;
    }

    return ret;
}

// 获取更新标志位的状态
u8 adc_get_update_flag(adc_channel_sel_t adc_channel)
{
    u8 ret = 0xFF;
    switch (adc_channel)
    {
    case ADC_CHANNEL_SEL_AD_KEY:
        ret = (u8)flag_is_adkey_val_update;
        break;
    case ADC_CHANNEL_SEL_BAT_DET:
        ret = (u8)flag_is_adc_bat_det_val_update;
        break;
    case ADC_CHANNEL_SEL_SOLAR_DET:
        ret = (u8)flag_is_adc_solar_det_val_update;
        break;

    default:
        break;
    }

    return ret;
}

// 清除更新标志位的状态
void adc_clear_update_flag(adc_channel_sel_t adc_channel)
{
    switch (adc_channel)
    {
    case ADC_CHANNEL_SEL_AD_KEY:
        flag_is_adkey_val_update = 0;
        break;
    case ADC_CHANNEL_SEL_BAT_DET:
        flag_is_adc_bat_det_val_update = 0;
        break;
    case ADC_CHANNEL_SEL_SOLAR_DET:
        flag_is_adc_solar_det_val_update = 0;
        break;

    default:
        break;
    }
}

void adc_channel_sel(adc_channel_sel_t adc_channel)
{
    switch (adc_channel)
    {
    case ADC_CHANNEL_SEL_AD_KEY:
        // ADC配置
        ADC_ACON1 &= ~(ADC_VREF_SEL(0x07) |  // 清空参考电压选择位
                       ADC_EXREF_SEL(0x01) | // 关闭外部参考电压
                       ADC_INREF_SEL(0x01)); // 关闭内部参考电压
        ADC_ACON1 |= ADC_VREF_SEL(0x06) |    // 选择 VCC 作为参考电压
                     ADC_TEN_SEL(0x03);      // 关闭测试信号
        ADC_CHS0 = ADC_ANALOG_CHAN(0x14);    // 选则引脚对应的通道（0x14--P24）
        break;
    case ADC_CHANNEL_SEL_BAT_DET:
        ADC_ACON1 &= ~(ADC_VREF_SEL(0x07) |  // 清空参考电压选择位
                       ADC_EXREF_SEL(0x01) | // 关闭外部参考电压
                       ADC_INREF_SEL(0x01)); // 关闭内部参考电压
        ADC_ACON1 |= ADC_VREF_SEL(0x01) |    // 选择 内部2.0V 作为参考电压
                     ADC_TEN_SEL(0x03) |     // 关闭测试信号
                     ADC_INREF_SEL(0x01);    // 使能内部参考电压
        ADC_CHS0 = ADC_EXT_SEL(0x01) |       // 选择内部通道
                   ADC_ANALOG_CHAN(0x03);    // 选择 VDD 1/5分压的通道
        break;
    case ADC_CHANNEL_SEL_SOLAR_DET:
        //  0 ~ 5 V， 经过二极管0.3V后，实际的充电电压范围：0 ~ 4.7V
        // 经过 1/2 分压后，单片机检测到的电压范围：0 ~ 2.35V
        // 使用2.4V参考电压
        ADC_ACON1 &= ~(ADC_VREF_SEL(0x07) |  // 清空参考电压选择位
                       ADC_EXREF_SEL(0x01) | // 关闭外部参考电压
                       ADC_INREF_SEL(0x01)); // 关闭内部参考电压
        ADC_ACON1 |= ADC_VREF_SEL(0x02) |    // 选择 内部2.4V 作为参考电压
                     ADC_TEN_SEL(0x03) |     // 关闭测试信号
                     ADC_INREF_SEL(0x01);    // 使能内部参考电压
        ADC_CHS0 = ADC_ANALOG_CHAN(0x0C);    // 选则引脚对应的通道（0x0C -- P14）
        break;

    default:
        break;
    }

    ADC_CFG0 |= ADC_CHAN0_EN(0x1) | // 使能通道0
                ADC_EN(0x1);        // 使能adc
}

// 由1ms及以上的定时器调用
void adc_scan(void)
{
    if (ADC_STATUS_IDLE == cur_adc_status ||
        ADC_STATUS_SEL_SOLAR_DET == cur_adc_status)
    {
        adc_channel_sel(ADC_CHANNEL_SEL_AD_KEY);
        cur_adc_status = ADC_STATUS_SEL_AD_KEY_WAITING;
    }
    else if (ADC_STATUS_SEL_AD_KEY_WAITING == cur_adc_status)
    {
        // 开启转换，之后在ad中断获取ad值
        ADC_CFG0 |= 0x01 << 0; // 开启 adc0 转换
        cur_adc_status = ADC_STATUS_SEL_AD_KEY;
    }
    else if (ADC_STATUS_SEL_AD_KEY == cur_adc_status)
    {
        adc_channel_sel(ADC_CHANNEL_SEL_BAT_DET);
        cur_adc_status = ADC_STATUS_SEL_BAT_DET_WAITING;
    }
    else if (ADC_STATUS_SEL_BAT_DET_WAITING == cur_adc_status)
    {
        // 开启转换，之后在ad中断获取ad值
        ADC_CFG0 |= 0x01 << 0; // 开启 adc0 转换
        cur_adc_status = ADC_STATUS_SEL_BAT_DET;
    }
    else if (ADC_STATUS_SEL_BAT_DET == cur_adc_status)
    {
        adc_channel_sel(ADC_CHANNEL_SEL_SOLAR_DET);
        cur_adc_status = ADC_STATUS_SEL_SOLAR_DET_WAITING;
    }
    else if (ADC_STATUS_SEL_SOLAR_DET_WAITING == cur_adc_status)
    {
        // 开启转换，之后在ad中断获取ad值
        ADC_CFG0 |= 0x01 << 0; // 开启 adc0 转换
        cur_adc_status = ADC_STATUS_SEL_SOLAR_DET;
    }
}

void ADC_IRQHandler(void) interrupt ADC_IRQn
{
    volatile u16 adc_val;

    // 进入中断设置IP，不可删除
    __IRQnIPnPush(ADC_IRQn);

    // ---------------- 用户函数处理 -------------------

    if (ADC_STA & ADC_CHAN0_DONE(0x01))
    {
        adc_val = (ADC_DATAH0 << 4) | (ADC_DATAL0 >> 4); // 先接收ad值
        if (ADC_STATUS_SEL_AD_KEY == cur_adc_status)
        {
            adc_update_val(ADC_CHANNEL_SEL_AD_KEY, adc_val);
        }
        else if (ADC_STATUS_SEL_BAT_DET == cur_adc_status)
        {
            adc_update_val(ADC_CHANNEL_SEL_BAT_DET, adc_val);
        }
        else if (ADC_STATUS_SEL_SOLAR_DET == cur_adc_status)
        {
            adc_update_val(ADC_CHANNEL_SEL_SOLAR_DET, adc_val);
        }

        ADC_STA |= ADC_CHAN0_DONE(0x01); // 清除ADC0转换完成标志位
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(ADC_IRQn);
}
