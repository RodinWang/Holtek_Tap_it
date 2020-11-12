#ifndef __MAIN_H
#define __MAIN_H

#include <stdlib.h>
#include "ht32.h"

#define MXIC_QIOR

#define TUBE_STOP()         {\
                              ww(0x4008830C, 4);\
                              while(1);\
                            }

#define CKCU_DBGCKR         rw(0x400880FC)

#define ENABLE_ISO()        HT_CKCU->LPCR &= ~(1UL << 0);
#define DISABLE_ISO()       HT_CKCU->LPCR |= (1UL << 0);

#define ENABLE_LSI()        HT_RTC->CR |= (1UL << 2)
#define DISABLE_LSI()       HT_RTC->CR &= ~(1UL << 2)

#define ENABLE_LSE()        HT_RTC->CR |= (1UL << 3)
#define DISABLE_LSE()       HT_RTC->CR &= ~(1UL << 3)

#define ENABLE_PLL()        HT_CKCU->GCCR |= (1UL << 9)
#define DISABLE_PLL()       HT_CKCU->GCCR &= ~(1UL << 9)

#define ENABLE_HSE()        HT_CKCU->GCCR |= (1UL << 10)
#define DISABLE_HSE()       HT_CKCU->GCCR &= ~(1UL << 10)

#define ENABLE_HSI()        HT_CKCU->GCCR |= (1UL << 11)
#define DISABLE_HSI()       HT_CKCU->GCCR &= ~(1UL << 11)

#define ENABLE_CKM()        HT_CKCU->GCCR |= (1UL << 16)
#define DISABLE_CKM()       HT_CKCU->GCCR &= ~(1UL << 16)

#define ENABLE_PSRC()       HT_CKCU->GCCR |= (1UL << 17)
#define DISABLE_PSRC()      HT_CKCU->GCCR &= ~(1UL << 17)

#define ENABLE_FLASH_SLEEP()  HT_CKCU->AHBCCR &= ~(1UL << 0)
#define DISABLE_FLASH_SLEEP() HT_CKCU->AHBCCR |= (1UL << 0)

#define ENABLE_SRAM_SLEEP()   HT_CKCU->AHBCCR &= ~(1UL << 2)
#define DISABLE_SRAM_SLEEP()  HT_CKCU->AHBCCR |= (1UL << 2)

#define ENABLE_BM_SLEEP()   HT_CKCU->AHBCCR &= ~(1UL << 5)
#define DISABLE_BM_SLEEP()  HT_CKCU->AHBCCR |= (1UL << 5)

#define ENABLE_APB_SLEEP()  HT_CKCU->AHBCCR &= ~(1UL << 6)
#define DISABLE_APB_SLEEP() HT_CKCU->AHBCCR |= (1UL << 6)

#define ENABLE_CKREF()      HT_CKCU->AHBCCR |= (1UL << 11)
#define DISABLE_CKREF()     HT_CKCU->AHBCCR &= ~(1UL << 11)

#define ENABLE_CKADC()      HT_CKCU->APBCCR1 |= (1UL << 24)
#define DISABLE_CKADC()     HT_CKCU->APBCCR1 &= ~(1UL << 24)

#define CHK_PLL_EN()        (HT_CKCU->GCCR & (1UL << 9))
#define CHK_HSE_EN()        (HT_CKCU->GCCR & (1UL << 10))
#define CHK_HSI_EN()        (HT_CKCU->GCCR & (1UL << 11))
#define CHK_PSRC_EN()       (HT_CKCU->GCCR & (1UL << 17))
#define CHK_LSI_EN()        (HT_RTC->CR & (1UL << 2))
#define CHK_LSE_EN()        (HT_RTC->CR & (1UL << 3))

#define CHK_PLL_RDY()       while ((HT_CKCU->GCSR & (1UL << 1)) == RESET)
#define CHK_HSE_RDY()       while ((HT_CKCU->GCSR & (1UL << 2)) == RESET)
#define CHK_HSI_RDY()       while ((HT_CKCU->GCSR & (1UL << 3)) == RESET)
#define CHK_LSE_RDY()       while ((HT_CKCU->GCSR & (1UL << 4)) == RESET)
#define CHK_LSI_RDY()       while ((HT_CKCU->GCSR & (1UL << 5)) == RESET)

#define CHK_FLASH_EN()      (HT_CKCU->AHBCCR & (1UL << 0))
#define CHK_SRAM_EN()       (HT_CKCU->AHBCCR & (1UL << 2))
#define CHK_BM_EN()         (HT_CKCU->AHBCCR & (1UL << 5))
#define CHK_APB_EN()        (HT_CKCU->AHBCCR & (1UL << 6))

#define SET_CKAHB_DIV(n)    HT_CKCU->AHBCFGR = n
#define SET_CKADC_DIV(n)    HT_CKCU->APBCFGR = (n << 16)
#define SET_CKREF_DIV(n)    HT_CKCU->GCFGR = (HT_CKCU->GCFGR & ~(31UL << 11)) | (n << 11)

#define SET_PLL_48M()       HT_CKCU->PLLCFGR = (6UL << 23) | (0UL << 21)
#define SET_PLL_24M()       HT_CKCU->PLLCFGR = (6UL << 23) | (1UL << 21)
#define SET_PLL_12M()       HT_CKCU->PLLCFGR = (6UL << 23) | (2UL << 21)
#define SET_PLL_6M()        HT_CKCU->PLLCFGR = (6UL << 23) | (3UL << 21)

#define SET_PLL_40M()       HT_CKCU->PLLCFGR = (5UL << 23) | (0UL << 21)
#define SET_PLL_20M()       HT_CKCU->PLLCFGR = (5UL << 23) | (1UL << 21)
#define SET_PLL_10M()       HT_CKCU->PLLCFGR = (5UL << 23) | (2UL << 21)
#define SET_PLL_5M()        HT_CKCU->PLLCFGR = (5UL << 23) | (3UL << 21)

#define SET_PLL_32M()       HT_CKCU->PLLCFGR = (4UL << 23) | (0UL << 21)
#define SET_PLL_16M()       HT_CKCU->PLLCFGR = (4UL << 23) | (1UL << 21)
#define SET_PLL_8M()        HT_CKCU->PLLCFGR = (4UL << 23) | (2UL << 21)
#define SET_PLL_4M()        HT_CKCU->PLLCFGR = (4UL << 23) | (3UL << 21)

//#define SET_PLL_24M()       HT_CKCU->PLLCFGR = (3UL << 23) | (0UL << 21)
//#define SET_PLL_12M()       HT_CKCU->PLLCFGR = (3UL << 23) | (1UL << 21)
//#define SET_PLL_6M()        HT_CKCU->PLLCFGR = (3UL << 23) | (2UL << 21)
#define SET_PLL_3M()        HT_CKCU->PLLCFGR = (3UL << 23) | (3UL << 21)

#define ENABLE_PLL_BYPASS()   HT_CKCU->PLLCR |= (1UL << 31)
#define DISABLE_PLL_BYPASS()  HT_CKCU->PLLCR &= ~(1UL << 31)

// Added dummy read to apply new wait-state setting into FSM @2015/01/09
#define SET_FLASH_0WS()     {HT_FLASH->CFCR = (HT_FLASH->CFCR & ~7UL) | 1UL; rw(0x00000010);}
#define SET_FLASH_1WS()     {HT_FLASH->CFCR = (HT_FLASH->CFCR & ~7UL) | 2UL; rw(0x00000010);}

#define ENABLE_CKS_INT()    HT_CKCU->GCIR |= (1UL << 16)
#define DISABLE_CKS_INT()   HT_CKCU->GCIR &= ~(1UL << 16)

#define __CK_PLL            1UL
#define __CK_HSE            2UL
#define __CK_HSI            3UL
#define __CK_LSE            6UL
#define __CK_LSI            7UL

#define SET_CKSYS_SRC(SRC)  HT_CKCU->GCCR = (HT_CKCU->GCCR & ~7UL) | (SRC);\
                            while ((HT_CKCU->CKST & 7UL) != SRC)

#define CHK_CKSYS_SRC(SRC)  ((HT_CKCU->CKST & 7UL) == SRC)

#define SET_CKPLL_SRC(SRC)  HT_CKCU->GCFGR = (HT_CKCU->GCFGR & ~(1UL << 8)) | ((SRC & 1UL) << 8)

#define CHK_CKPLL_SRC(SRC)  (((HT_CKCU->GCFGR >> 8) & 1UL) == (SRC & 1UL))

#define BY_CKSYS            1UL
#define BY_PLL              2UL
#define BY_CKM              4UL
#define BY_CKREF            8UL

#define CHK_PLL_USE(USE)    (HT_CKCU->CKST & (USE << 8))
#define CHK_HSE_USE(USE)    (HT_CKCU->CKST & (USE << 16))
#define CHK_HSI_USE(USE)    (HT_CKCU->CKST & (USE << 24))

#define CKO_CKREF           0UL
#define CKO_HCLKC_DIV16     1UL
#define CKO_CKSYS_DIV16     2UL
#define CKO_CKHSE_DIV16     3UL
#define CKO_CKHSI_DIV16     4UL
#define CKO_CKLSE           5UL
#define CKO_CKLSI           6UL
#define CKO_DBGCK_DIV16     7UL

#define SET_CKOUT_SRC(SRC)  HT_CKCU->GCFGR = (HT_CKCU->GCFGR & ~7UL) | (SRC)

#define PDMA_MIDI_OUT             PDMA_CH4                      /*!< MIDI_OUT PDMA channel number           */
#define PDMA_MIDI_IN              PDMA_CH3                      /*!< MIDI_IN PDMA channel number            */


#endif /* __MAIN_H */
