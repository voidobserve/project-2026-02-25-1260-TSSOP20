#include "timer1.h"
#include "user_include.h"

#define PEROID_VAL (SYSCLK / 1 / 10000 - 1) // 周期值=系统时钟/分频/频率 - 1

/**
 * @brief 配置定时器 TMR 1
 */
void timer1_init(void)
{
#if 1
    __EnableIRQ(TMR1_IRQn); // 使能 中断
    IE_EA = 1;              // 使能总中断

    TMR_ALLCON = TMR1_CNT_CLR(0x1);                        // 清除计数值
    TMR1_PRH = TMR_PERIOD_VAL_H((PEROID_VAL >> 8) & 0xFF); // 周期值
    TMR1_PRL = TMR_PERIOD_VAL_L((PEROID_VAL >> 0) & 0xFF);
    TMR1_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1); // 计数等于周期时允许发生中断
    TMR1_CONL = TMR_SOURCE_SEL(0x7) |                   // 选择系统时钟
                TMR_PRESCALE_SEL(0x00) |                // 预分频为1
                TMR_MODE_SEL(0x1);                      // 计数模式
#endif
}

// extern void fun(void);
// 定时器TMR0中断服务函数
void TIMR1_IRQHandler(void) interrupt TMR1_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR1_IRQn);

    // ---------------- 用户函数处理 -------------------
    // 周期中断
    if (TMR1_CONH & TMR_PRD_PND(0x1))
    {
        TMR1_CONH |= TMR_PRD_PND(0x1); // 清除pending 

        
        detect_1khz_signal_100us(); // 新增：每100us调用一次1KHz信号检测函数
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR1_IRQn);
}
