#include "i2c.h"

//Attention: IIC not add timeout function in this version,please use with caution
void I2C0_Init(void)
{
  I2C_InitTypeDef  I2C_InitStructure;
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
  INTERFACE_I2C_CLOCK(CKCUClock) = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
  /*  I2C0 configuration:
      Mode = Master,
      AddressingMode = 7BIT,
      Speed = ClockSpeed,
      SpeedOffset = 0
  */
  I2C_InitStructure.I2C_GeneralCall = DISABLE;
  I2C_InitStructure.I2C_AddressingMode = I2C_ADDRESSING_7BIT;
  I2C_InitStructure.I2C_Acknowledge = DISABLE;
  I2C_InitStructure.I2C_OwnAddress = I2C_MASTER_ADDRESS;
  I2C_InitStructure.I2C_Speed = ClockSpeed;
  I2C_InitStructure.I2C_SpeedOffset = 0;
  I2C_Init(INTERFACE_I2C, &I2C_InitStructure);
  
  I2C_Cmd(INTERFACE_I2C, ENABLE);
}

void InterfaceI2C_SendBuffer(u8* buff, u8 len, u8 slave_addr)
{
  u8 Tx_Index = 0;
  /* Send I2C START & I2C slave address for write                                                           */
  I2C_TargetAddressConfig(INTERFACE_I2C, slave_addr, I2C_MASTER_WRITE);

  /* Check on Master Transmitter STA condition and clear it                                                 */
  while (!I2C_CheckStatus(INTERFACE_I2C, I2C_MASTER_SEND_START));

  /* Check on Master Transmitter ADRS condition and clear it                                                */
  while (!I2C_CheckStatus(INTERFACE_I2C, I2C_MASTER_TRANSMITTER_MODE));
  /* Send data                                                                                              */
  while (Tx_Index < len)
  {
    /* Check on Master Transmitter TXDE condition                                                           */
    while (!I2C_CheckStatus(INTERFACE_I2C, I2C_MASTER_TX_EMPTY));
    /* Master Send I2C data                                                                                 */
    I2C_SendData(INTERFACE_I2C, buff[Tx_Index++]);
  }
  /* Send I2C STOP condition                                                                                */
  I2C_GenerateSTOP(INTERFACE_I2C);
  /*wait for BUSBUSY become idle                                                                            */
  while (I2C_ReadRegister(INTERFACE_I2C, I2C_REGISTER_SR)&0x80000);
}

void InterfaceI2C_ReceiveBuffer(u8* buff, u8 len, u8 slave_addr)
{
  u8 Rx_Index = 0;
  /* Send I2C START & I2C slave address for read                                                            */
  I2C_TargetAddressConfig(INTERFACE_I2C, slave_addr, I2C_MASTER_READ);

  /* Check on Master Transmitter STA condition and clear it                                                 */
  while (!I2C_CheckStatus(INTERFACE_I2C, I2C_MASTER_SEND_START));

  /* Check on Master Transmitter ADRS condition and clear it                                                */
  while (!I2C_CheckStatus(INTERFACE_I2C, I2C_MASTER_RECEIVER_MODE));

  I2C_AckCmd(INTERFACE_I2C, ENABLE);
  /* Send data                                                                                              */
  while (Rx_Index < len)
  {

    /* Check on Slave Receiver RXDNE condition                                                              */
    while (!I2C_CheckStatus(INTERFACE_I2C, I2C_MASTER_RX_NOT_EMPTY));
    /* Store received data                                                                         */
    buff[Rx_Index++] = I2C_ReceiveData(INTERFACE_I2C);
    if (Rx_Index == (len-1))
    {
      I2C_AckCmd(INTERFACE_I2C, DISABLE);
    }
  }
  /* Send I2C STOP condition                                                                                */
  I2C_GenerateSTOP(INTERFACE_I2C);
  /*wait for BUSBUSY become idle                                                                            */
  while (I2C_ReadRegister(INTERFACE_I2C, I2C_REGISTER_SR)&0x80000);
}
