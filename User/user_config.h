#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "include.h" // 芯片官方提供的头文件

// =========================================================================================================================
 
#define USER_DEBUG_ENABLE 1 // 是否使用打印调试
#if USER_DEBUG_ENABLE
#include <stdio.h>
#include "user_test.h"
/*
    测试引脚： 
*/
// #define USE_MY_TEST_PIN 0 // 是否使用测试用的引脚（开发板没有相关的引脚，用其他空闲的引脚来代替）

#endif // USER_DEBUG_ENABLE
// =========================================================================================================================

#define ARRAY_SIZE(n) (sizeof(n) / sizeof(n[0]))
 

#endif
