# charge_det.c

- detect_1khz_signal_100us()
在定时器中断内调用，100us检测一次
有1Khz信号并且持续10ms，则认为正在通过type-c充电
没有1Khz信号并且累计2s，则认为不在通过type-c充电

- charge_det()
连续200ms没有检测到充电器一侧的电压，则认为不在通过type-c充电

# low_power.c
P24 ad按键检测脚 唤醒通道0，有低电平则触发唤醒
P21 充电ic检测脚CH1 唤醒通道1，有高电平触发唤醒
WUT 唤醒通道2，有高电平触发唤醒

唤醒之后，会判断以下条件是否有成立的，才退出低功耗
- ad按键是否按下
- 太阳能一侧是否大于4.5V
- 充电icCH1是否正在输出充电的信号
如果都不成立，则回到低功耗


---

编译之后，编译器会提示如下警告：
```text
*** WARNING L15: MULTIPLE CALL TO SEGMENT
    SEGMENT: ?PR?_GET_BATTERY_PERCENTAGE_BY_VOLTAGE?BATTERY_MONITOR
    CALLER1: ?C_C51STARTUP
    CALLER2: ?PR?ADC_IRQHANDLER?ADC
```
可以不用理会，get_battery_percentage_by_voltage()函数一个是在低功耗刚唤醒，要不要再回到低功耗的判断期间会调用，另一个是在adc中断内调用。两种调用不会同时调用产生冲突
  