#ifndef __BLEUSER_H
#define __BLEUSER_H

#include "ht32.h"
#include "buffer.h"
#include "bc76xx.h"
#include "bleprocess.h"

#define   BLE_TX_Size    (128)
#define   BLE_RX_Size    (64)

extern u8 BLE_rx_buff[BLE_RX_Size + 1];
extern Buffer_TypeDef BLE_rx_index;

u8 Get_BLE_RX_Length(void);
u8 Get_BLE_TX_Length(void);
u8 BLE_Read_Buffer(u8* buff, u8 len);
u8 BLE_Write_Buffer(u8* buff, u8 len);
void BLE_Set_Name(const char* buff);
void BLE_Set_Address(u8* buff);
void BLE_Set_Advertise(u32 time);
void BLE_Power_Down(void);
void BLE_Power_Up(void);
u8 Get_BLE_Status(void);
bool BLE_Advertise_Data(u8 len, u8* buffer);
void Stop_Advertise(void);
u32 Get_Advertise_Time(void);
#endif

