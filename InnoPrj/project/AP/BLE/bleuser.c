#include "bleuser.h"
#include "bc76xx.h"
#include "bleprocess.h"
#include <string.h>

void BLE_param_configure(void);
void BLE_enter_power_save(void);
void BLE_enter_power_up(void);

u8 BLE_rx_buff[BLE_RX_Size + 1];
Buffer_TypeDef BLE_rx_index;

/*********************************************************************************************************//**
 * @brief Get valid receive data length
 * @param None
 * @retval The length of valid data
 ************************************************************************************************************/
u8 Get_BLE_RX_Length(void)
{
  return Get_Valid_Lenth(&BLE_rx_index);
}

/*********************************************************************************************************//**
 * @brief Get valid send buffer data length
 * @param None
 * @retval The length of valid data
 ************************************************************************************************************/
u8 Get_BLE_TX_Length(void)
{
  return transmit_data_len;
}

/*********************************************************************************************************//**
 * @brief Read BLE buffer
 * @param buffer: the buffer to receive
 * @param len: the number to read
 * @retval The reality number read
 ************************************************************************************************************/
u8 BLE_Read_Buffer(u8* buff, u8 len)
{
  return Buffer_Read(&BLE_rx_index, buff, len);
}

/*********************************************************************************************************//**
 * @brief Write BLE buffer
 * @param buffer: the buffer to write
 * @param len: the number to write
 * @retval The reality number written
 ************************************************************************************************************/
u8 BLE_Write_Buffer(u8* buff, u8 len)
{
  u8 len_temp = 0;
  
  if(transmit_data_len + len < BLE_TX_Size)
  {
    memcpy(&(((MESSAGE_PACKAGE *)transmit_data)->data[transmit_data_len]), buff, len);
    transmit_data_len += len;
    return len;
  }
  else
  {
    memcpy(&(((MESSAGE_PACKAGE *)transmit_data)->data[transmit_data_len]), buff, BLE_TX_Size - transmit_data_len);
    len_temp = BLE_TX_Size - transmit_data_len;
    transmit_data_len = BLE_TX_Size;
    return len_temp;
  }
}

/*********************************************************************************************************//**
 * @brief Set BLE device name
 * @param buffer: name string,max size = 16byte
 * @retval None
 ************************************************************************************************************/
void BLE_Set_Name(const char* buff)
{
	u8 TX_data[19];
  extern u8	BLE_BD_Name[16];
	
	TX_data[0] = 0x25;
	TX_data[1] = 0x31;
	TX_data[2] = strlen(buff);
	if(TX_data[2] > 16)
		return;
  memcpy(&TX_data[3], buff, TX_data[2]);
  
  memset(BLE_BD_Name, 0, 16);
  memcpy(BLE_BD_Name, buff, TX_data[2]);
  
  BC76xxSPI_write_fifo(TX_data, TX_data[2]+3);
}

/*********************************************************************************************************//**
 * @brief Set BLE advertise time interval
 * @param time: 20~10000ms
 * @retval None
 ************************************************************************************************************/
void BLE_Set_Advertise(u32 time)
{
	u8 TX_data[7];
	
	TX_data[0] = 0x25;
	TX_data[1] = 0x35;
	TX_data[2] = 4;
  
  memcpy(&TX_data[3], &time, TX_data[2]);
  
  BC76xxSPI_write_fifo(TX_data, TX_data[2]+3);  
}

/*********************************************************************************************************//**
 * @brief Set BLE power down
 * @param None
 * @retval None
 ************************************************************************************************************/
void BLE_Power_Down(void)
{
  BLE_param_configure();
  BLE_enter_power_save();
}

/*********************************************************************************************************//**
 * @brief Set BLE power up
 * @param None
 * @retval None
 * @note BLE device will initialize again
 ************************************************************************************************************/
void BLE_Power_Up(void)
{
  BLE_enter_power_up();
  Buffer_Discard(&BLE_rx_index);
}

/*********************************************************************************************************//**
 * @brief Write BLE buffer
 * @param None
 * @retval BLE status flag
 ************************************************************************************************************/
u8 Get_BLE_Status(void)
{
  switch(BLEoperate.state)
  {
    case _BLE_PWR_ON_SUCCESS_:
      return 0x00;
    case _BLE_PWR_ON_ERROR_:
      return 0x01;
    case _BLE_POWER_DOWN_:
      return 0x02;
    case _BLE_ADVERTISE_:
      return 0x03;
    default:
      return 0x04;
  }
}

/*********************************************************************************************************//**
 * @brief Write BLE buffer
 * @param buffer: the buffer to advertise
 * @param len: the number to advertise
 * @retval Succeed or Failed
 ************************************************************************************************************/
bool BLE_Advertise_Data(u8 len, u8* buffer)
{
  u8 temp_buf[32];
  u8 adv_index = 0;
  extern u8	BLE_BD_Name[16];
  
  switch(BLEoperate.adver_event.adver_stage)
  {
    case _ADVER_IDLE_:
      if(BC76xx_get_wakeup() == HIGH)
      {
        BC76xx_wakeup(LOW);
        BLEoperate.adver_event.adver_cnt = 0;
        BLEoperate.adver_event.adver_stage = _ADVER_START_;
        BLEoperate.adver_event.updateAdver_flag = TRUE; 
        BLEoperate.state = _BLE_ADVERTISE_; 
      }
      break;
    
    case _ADVER_START_: 
    case _ADVER_DELAY_:
      return FALSE;
    
    case _ADVER_WORKING_:
      BLEoperate.adver_event.adver_cnt = 0;
      BLEoperate.adver_event.updateAdver_flag = TRUE;
      break;
  }
  temp_buf[adv_index++] = 0x02;
  temp_buf[adv_index++] = 0x01;
  temp_buf[adv_index++] = 0x06;
  
  temp_buf[adv_index++] = strlen((const char*)BLE_BD_Name) + 1;
  temp_buf[adv_index++] = 0x09;
  memcpy(&temp_buf[adv_index], BLE_BD_Name, temp_buf[3]);
  adv_index += temp_buf[3] - 1;
  
  temp_buf[adv_index++] = len + 1;
  temp_buf[adv_index++] = 0xFF;
  memcpy(&temp_buf[adv_index], buffer, len);
  adv_index += len;
  
  memcpy(BLEoperate.adver_event.adver_buffer, temp_buf, adv_index);
  BLEoperate.adver_event.adver_len = adv_index;
  
  return TRUE;
}

/*********************************************************************************************************//**
 * @brief Get BLE davertise times since this operaion
 * @param None
 * @retval Times
 ************************************************************************************************************/
u32 Get_Advertise_Time(void)
{
  return BLEoperate.adver_event.adver_cnt;
}

/*********************************************************************************************************//**
 * @brief Stop BLE advertise
 * @param None
 * @retval None
 ************************************************************************************************************/
void Stop_Advertise(void)
{
  BC76xx_wakeup(HIGH);
  BLEoperate.adver_event.adver_stage = _ADVER_IDLE_;
  BLEoperate.state = _BLE_PWR_ON_SUCCESS_;
}

//默認設定傳輸閘口時間為10~20ms
void BLE_Test(void)
{
	u8 TX_data[19];
	
	TX_data[0] = 0x25;
	TX_data[1] = 0x40;
  TX_data[2] = 0x08;
  TX_data[3] = 0x08;
  TX_data[4] = 0x00;
  TX_data[5] = 0x10;
	TX_data[6] = 0x00;
  TX_data[7] = 0x00;
  TX_data[8] = 0x00;
  TX_data[9] = 0x58;
  TX_data[10] = 0x02;
  
  BC76xxSPI_write_fifo(TX_data, TX_data[2]+3);
}
