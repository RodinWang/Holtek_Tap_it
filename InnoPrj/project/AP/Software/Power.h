#ifndef __POWER_H
#define __POWER_H

#include "ht32.h"
#include "gpio.h"
#include "qspi_flash_MX25L12835F.h"
#include "drv_ht82v73a.h"
#include "bleuser.h"
#include "urbus.h"

void LowPower_Init(void);
void Bus_SystemResetAll(void);
void Bus_SystemStandby(void);
void Bus_SystemAction(void);

#define Halt()                        (PWRCU_DeepSleep1(PWRCU_SLEEP_ENTRY_WFE))
#define Enter_LowPower()              {LowPower_Init();Halt();}

#define HS_BUS_RX_AFIO_EXTI_CH        (AFIO_EXTI_CH_1)
#define HS_BUS_RX_EXTI_ESS            (AFIO_ESS_PB)
#define HS_BUS_RX_EXTI_CH             (EXTI_CHANNEL_1)

#define LS_BUS_RX_AFIO_EXTI_CH        (AFIO_EXTI_CH_5)
#define LS_BUS_RX_EXTI_ESS            (AFIO_ESS_PC)
#define LS_BUS_RX_EXTI_CH             (EXTI_CHANNEL_5)

#define HV_BUS_RX_AFIO_EXTI_CH        (AFIO_EXTI_CH_3)
#define HV_BUS_RX_EXTI_ESS            (AFIO_ESS_PA)
#define HV_BUS_RX_EXTI_CH             (EXTI_CHANNEL_3)

#define ENTER_DELAY                   (50)//ms
#define PERIOD_115200                 (8)//us
#define PERIOD_9600                   (104)//us

#endif
