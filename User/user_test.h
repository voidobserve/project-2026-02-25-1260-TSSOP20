#ifndef __USER_TEST_H__
#define __USER_TEST_H__

#include "include.h"
#include "my_config.h"
#if USE_MY_DEBUG

extern volatile u8 flag_debug;

void debug_time_add(void);

#endif

#endif
