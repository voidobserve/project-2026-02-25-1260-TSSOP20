# charge_det.c

- `charge_det.c` 充电ic检测，会检测充电ic输出的充电信号和放电信号
 

# low_power.c
P24 ad按键检测脚 唤醒通道0，有低电平则触发唤醒
P21 充电ic检测脚CH1 唤醒通道1，有高电平触发唤醒
WUT 唤醒通道2，有高电平触发唤醒 

# bat_scan.c

- `bat_scan.c` 只检测电池电压，引出相关接口


---

编译之后，编译器会提示如下警告：
```text
*** WARNING L15: MULTIPLE CALL TO SEGMENT
    SEGMENT: ?PR?_GET_BATTERY_PERCENTAGE_BY_VOLTAGE?BATTERY_MONITOR
    CALLER1: ?C_C51STARTUP
    CALLER2: ?PR?ADC_IRQHANDLER?ADC
```
可以不用理会，get_battery_percentage_by_voltage()函数一个是在低功耗刚唤醒，要不要再回到低功耗的判断期间会调用，另一个是在adc中断内调用。两种调用不会同时调用产生冲突
  