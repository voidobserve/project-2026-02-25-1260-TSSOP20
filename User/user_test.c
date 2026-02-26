#include "user_test.h"
#include <stdio.h>

#include "adc.h"

#if USE_MY_DEBUG

volatile u8 flag_debug = 0;

void debug_time_add(void)
{
    static u16 cnt = 0;
    cnt++;
    if (cnt >= 1000)
    {
        cnt = 0;

        flag_debug = 1;
    }
}

// if (adc_get_update_flag(ADC_CHANNEL_SEL_AD_KEY))
// {
//     adc_clear_update_flag(ADC_CHANNEL_SEL_AD_KEY);
//     adkey_val = adc_get_val(ADC_CHANNEL_SEL_AD_KEY);
//     printf("adkey_val == %u\n", adkey_val);
//     delay_ms(500);
// }

// printf("adkey 1 press val == %u\n", AD_KEY_1_PRESS_VAL);
// printf("adkey 2 press val == %u\n", AD_KEY_2_PRESS_VAL);
// printf("adkey 3 press val == %u\n", AD_KEY_3_PRESS_VAL);
// printf("adkey 4 press val == %u\n", AD_KEY_4_PRESS_VAL);
// printf("adkey index 1 val == %u\n", AD_KEY_INDEX_1_VAL);
// printf("adkey index 2 val == %u\n", AD_KEY_INDEX_2_VAL);
// printf("adkey index 3 val == %u\n", AD_KEY_INDEX_3_VAL);
// printf("adkey index 4 val == %u\n", AD_KEY_INDEX_4_VAL);
// delay_ms(500);

void user_test_adc_bat_scan(void)
{
    u16 adkey_val = 0;
    if (adc_get_update_flag(ADC_CHANNEL_SEL_BAT_DET) && flag_debug)
    {
        flag_debug = 0;
        adc_clear_update_flag(ADC_CHANNEL_SEL_BAT_DET);
        adkey_val = adc_get_val(ADC_CHANNEL_SEL_BAT_DET);
        printf("adkey_val == %u\n", adkey_val);
        // delay_ms(500);
    }
}

#endif
