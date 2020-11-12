#ifndef __ADC_H
#define __ADC_H

#include "ht32.h"

#define INTERFACE_ADC_CLOCK(CK)   (CK.Bit.ADC)

void InterfaceADC_Init(void);
void InterfaceADC_StartConversion(void);
#endif
