#include "led.h"

void led_init(void)
{
    // 100% 电量指示灯
    P1_MD0 &= ~GPIO_P13_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P13_MODE_SEL(0x01); // 输出
    FOUT_S13 = GPIO_FOUT_AF_FUNC;
    P13 = 0; 

    // 75% 电量指示灯
    P0_MD1 &= ~GPIO_P06_MODE_SEL(0x03);
    P0_MD1 |= GPIO_P06_MODE_SEL(0x01);
    FOUT_S06 = GPIO_FOUT_AF_FUNC;
    P06 = 0; // 不点亮指示灯

    // 50% 电量指示灯
    P0_MD1 &= ~GPIO_P05_MODE_SEL(0x03);
    P0_MD1 |= GPIO_P05_MODE_SEL(0x01);
    FOUT_S05 = GPIO_FOUT_AF_FUNC;
    P05 = 0; // 不点亮指示灯

    // 25% 电量指示灯
    P0_MD0 &= ~GPIO_P00_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P00_MODE_SEL(0x01);
    FOUT_S00 = GPIO_FOUT_AF_FUNC;
    P00 = 0; // 不点亮指示灯 
}
