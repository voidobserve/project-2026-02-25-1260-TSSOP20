# charge_det.c

- detect_1khz_signal_100us
在定时器中断内调用，100us检测一次
有1Khz信号并且持续10ms，则认为正在通过type-c充电
没有1Khz信号并且累计2s，则认为不在通过type-c充电