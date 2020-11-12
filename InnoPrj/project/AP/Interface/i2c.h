#ifndef __I2C_H
#define __I2C_H

#include "ht32.h"

#define INTERFACE_I2C             (HT_I2C0)
#define INTERFACE_I2C_CLOCK(CK)   (CK.Bit.I2C0)

#define I2C_MASTER_ADDRESS     0x0A
#define ClockSpeed             1000000

void I2C0_Init(void);
void InterfaceI2C_SendBuffer(u8* buff, u8 len, u8 slave_addr);
void InterfaceI2C_ReceiveBuffer(u8* buff, u8 len, u8 slave_addr);
#endif
