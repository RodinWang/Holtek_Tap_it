#ifndef __SYS_TIMER_H
#define __SYS_TIMER_H

#include "ht32.h"

#define	CFG_SYSTEM_TM0						SCTM0
#define	CFG_SYSTEM_TM1						SCTM1

#define SYSTEM_TM0                STRCAT2(HT_,              CFG_SYSTEM_TM0)
#define SYSTEM_TM0_IRQn           STRCAT2(CFG_SYSTEM_TM0,   _IRQn)
#define SYSTEM_TM0_IRQHandler     STRCAT2(CFG_SYSTEM_TM0,   _IRQHandler)
#define SYSTEM_TM1                STRCAT2(HT_,              CFG_SYSTEM_TM1)
#define SYSTEM_TM1_IRQn           STRCAT2(CFG_SYSTEM_TM1,   _IRQn)
#define SYSTEM_TM1_IRQHandler     STRCAT2(CFG_SYSTEM_TM1,   _IRQHandler)

#define SYSTEM_TIMER_SET          (1000)   //us

void Timer_Init(void);
#endif
