#include "urbus.h"
#include "iap_handler.h"
#include "key_func.h"
#include "bleuser.h"
#include "bc76xx.h"
#include "power.h"
#include <string.h>

HT_USART_TypeDef* BUS_Port_table[3] = {LS_BUS_PORT, HS_BUS_PORT, HV_BUS_PORT};
/* public buffer defination */
u8 VCP_RX_Buffer[VCP_RX_BUFFER_SIZE + BUFFER_ADDITIONAL_SIZE];
Buffer_TypeDef VCP_RX_Index;//VCP_RX
u8 BUS_RX_Buffer[3][64 + BUFFER_ADDITIONAL_SIZE];
Buffer_TypeDef BUS_RX_Index[3];

u8 Bus_send_buffer[BUS_TX_BUFFER_SIZE];
u16 Bus_send_index = 0;//bus send
u8 VCP_send_buffer[VCP_TX_BUFFER_SIZE];
u16 VCP_send_index = 0;//vcp send

u16 busRXCount[3];

static USART0_USING_ENUM USR0_using_flag = USART0_IDLE;
static u8 UR_Bus_response_flag = {0};
static vu16 Bus_Timeout_Value[3];
/* private function */
#define Bus_Set_Timeout(bus, nms) (Bus_Timeout_Value[bus] = nms*1000/BFTM_TIME_BASE)
#define Check_Bus_Timeout(bus)    (Bus_Timeout_Value[bus] == 0)
static void Parse_Bus_Cmd(void);
static void Parse_Voice_Cmd(void);
static bool Parse_Retval(u8 num);

/*********************************************************************************************************//**
 * @brief Initialize UR Bus,include COLOCK/BFTM1/RX_BUFFER/IO and UART
 * @param None
 * @retval None
 ************************************************************************************************************/
void URBus_Init(void)
{
  USART_InitTypeDef USART_InitStructure;
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
  CKCUClock.Bit.BFTM1    = 1;
  HS_BUS_CLK(CKCUClock)  = 1;
  LS_BUS_CLK(CKCUClock)  = 1;
  HV_BUS_CLK(CKCUClock)  = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
  
  Buffer_Init(&BUS_RX_Index[LS_BUS_NUM], BUS_RX_Buffer[LS_BUS_NUM], 64+BUFFER_ADDITIONAL_SIZE);
  Buffer_Init(&BUS_RX_Index[HS_BUS_NUM], BUS_RX_Buffer[HS_BUS_NUM], 64+BUFFER_ADDITIONAL_SIZE);
  Buffer_Init(&BUS_RX_Index[HV_BUS_NUM], BUS_RX_Buffer[HV_BUS_NUM], 64+BUFFER_ADDITIONAL_SIZE);
  
  BFTM_SetCompare(HT_BFTM1, SystemCoreClock / 1000000 * BFTM_TIME_BASE);
  BFTM_OneShotModeCmd(HT_BFTM1, DISABLE);
  BFTM_IntConfig(HT_BFTM1, ENABLE);
  BFTM_EnaCmd(HT_BFTM1, ENABLE);
  NVIC_EnableIRQ(BFTM1_IRQn);
  
//  AFIO_GPxConfig(HS_BUS_TX_GPIO_ID, HS_BUS_TX_AFIO_PIN, HS_BUS_TX_AFIO_MODE);
//  AFIO_GPxConfig(HS_BUS_RX_GPIO_ID, HS_BUS_RX_AFIO_PIN, HS_BUS_RX_AFIO_MODE);
  GPIO_OpenDrainConfig(HS_BUS_TX_PORT, HS_BUS_TX_GPIO_PIN, ENABLE);
  AFIO_GPxConfig(LS_BUS_TX_GPIO_ID, LS_BUS_TX_AFIO_PIN, LS_BUS_TX_AFIO_MODE);
  AFIO_GPxConfig(LS_BUS_RX_GPIO_ID, LS_BUS_RX_AFIO_PIN, LS_BUS_RX_AFIO_MODE);
  GPIO_OpenDrainConfig(LS_BUS_TX_PORT, LS_BUS_TX_GPIO_PIN, ENABLE); 
//  AFIO_GPxConfig(HV_BUS_TX_GPIO_ID, HV_BUS_TX_AFIO_PIN, HV_BUS_TX_AFIO_MODE);
//  AFIO_GPxConfig(HV_BUS_RX_GPIO_ID, HV_BUS_RX_AFIO_PIN, HV_BUS_RX_AFIO_MODE); 
  GPIO_OpenDrainConfig(HV_BUS_TX_PORT, HV_BUS_TX_GPIO_PIN, ENABLE);
  
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WORDLENGTH_9B;
  USART_InitStructure.USART_StopBits = USART_STOPBITS_1;
  USART_InitStructure.USART_Parity = USART_PARITY_NO;
  USART_InitStructure.USART_Mode = USART_MODE_NORMAL;

  USART_Init(HS_BUS_PORT, &USART_InitStructure);
  USART_Init(HV_BUS_PORT, &USART_InitStructure);
  USART_InitStructure.USART_BaudRate = 9600;
  USART_Init(LS_BUS_PORT, &USART_InitStructure);
  
  USART_IntConfig(HS_BUS_PORT, USART_INT_RXDR, ENABLE);
  NVIC_EnableIRQ(HS_BUS_IRQn);
  USART_IntConfig(LS_BUS_PORT, USART_INT_RXDR, ENABLE);
  NVIC_EnableIRQ(LS_BUS_IRQn);
  USART_IntConfig(HV_BUS_PORT, USART_INT_RXDR, ENABLE);
  NVIC_EnableIRQ(HV_BUS_IRQn);   
}

/*********************************************************************************************************//**
 * @brief UR BUS bottom driver send function
 * @param data: u16 only low 9bit is valid
 * @param buffer: the buffer to be send, only low 8bit is valid, the 9st bit fixed 0
 * @retval None
 ************************************************************************************************************/
void Uart_Send_Data(u8 num, u16 data)
{
  USART_TxCmd(BUS_Port_table[num], ENABLE);//enable send function
  while ((BUS_Port_table[num]->SR & USART_FLAG_TXDE) == 0); 
  BUS_Port_table[num]->DR = data;
  while ((BUS_Port_table[num]->SR & USART_FLAG_TXC) == 0);  
  USART_TxCmd(BUS_Port_table[num], DISABLE);//disable send function
}
void Uart_Send_Buffer(u8 num, u8* buffer, u32 length)
{
  USART_TxCmd(BUS_Port_table[num], ENABLE);////enable send function
  while(length--)
  {
    while ((BUS_Port_table[num]->SR & USART_FLAG_TXDE) == 0); 
    BUS_Port_table[num]->DR = *buffer;
    while ((BUS_Port_table[num]->SR & USART_FLAG_TXC) == 0); 
    buffer++;
  }
  USART_TxCmd(BUS_Port_table[num], DISABLE);////disable send function
}
/* end of driver function*/

/*********************************************************************************************************//**
 * @brief Parse the command packet from terminal used VCP
 * @param None
 * @retval None
 ************************************************************************************************************/
static void Parse_Bus_Cmd(void)
{
  /*V1.0 上位機發cmd+mid+eid+instru+para, MCU打包幀格式
  u8 length;
  u32 check_sum = 0;
  
  length = Get_Valid_Lenth(&VCP_RX_Index);
  length += 1;//all frame length
  
  Buffer_Read(&VCP_RX_Index, &Bus_send_buffer[Bus_send_index], 2);//MID+EID
  Bus_send_index += 2;
  if(length > 15)
  {
    Bus_send_buffer[Bus_send_index] = length-3+1;
    Bus_send_index++;
  }else
  {
    Bus_send_buffer[1] |= (length-3+1) << 4;
  }
  Buffer_Read(&VCP_RX_Index, &Bus_send_buffer[Bus_send_index], length-3);//last all,Insru+Para
  for(u8 i = 0;i < length-3;i++)
  {
    check_sum+=Bus_send_buffer[Bus_send_index++];
  }
  check_sum = ~check_sum;
  Bus_send_buffer[Bus_send_index] = (u8)check_sum;
  Bus_send_index++; 
  */ 
  
  /*V2.0 上位機打包完整，發cmd+mid+eid_tlen+(len2)+instru+para+cs
  MCU要做的工作：
  1. 不需要任何校驗，因為USB傳輸會進行校驗
  2. MID最高位置1，其他最高位保持為0，以9bit格式bypass
  3. 將VCP_RX_Buffer數據原樣copy至Bus_send_buffer，Bus_send_index遞增
  */
  Bus_send_index = Get_Valid_Lenth(&VCP_RX_Index);
  Buffer_Read(&VCP_RX_Index, Bus_send_buffer, Bus_send_index);
}

static void Parse_Voice_Cmd(void)
{
  u8 temp_buffer[16];
  u8 length;
  u32 voice_index;
  
  Buffer_Read(&VCP_RX_Index, temp_buffer, 2);//uid not care
  if(temp_buffer[1] & 0xF0)
  {
    length = temp_buffer[1] >> 4;
  }else
  {
    Buffer_Read(&VCP_RX_Index, &length, 1);
  }
  
  Buffer_Read(&VCP_RX_Index, temp_buffer, length);
  switch(temp_buffer[0])
  {
    case 0x08:
      voice_index = temp_buffer[2] | temp_buffer[3]<<8 | temp_buffer[4]<<16 | temp_buffer[5]<<24;
			if((temp_buffer[1] == 1 && voice_index > c_song_max) || (temp_buffer[1] == 2 && voice_index > c_voice_max))
			{
				goto Voice_Error;
			}
      L_Audio_Play(temp_buffer[1], voice_index);
      break;
    
    case 0x09:
      L_Audio_Vol(temp_buffer[1]);
      break;
    
    case 0x0A:
      if(L_Audio_Finish() == 0)
      {
        L_Audio_Stop();
        break;
      }
      else
      {
        goto Voice_Error;
      }
			
    case 0x0B:
      if(L_Audio_Finish())
      {
        goto Voice_Error;
      }
      break;  
  }
  VCP_send_buffer[0] = 0x00;
  VCP_send_buffer[1] = 0x20;
  VCP_send_buffer[2] = 0x00;
  VCP_send_buffer[3] = 0xDF;
  VCP_send_index = 4;
	return;
Voice_Error:
	VCP_send_buffer[2] = 0x01;
	VCP_send_buffer[3] = 0xDE;
	VCP_send_buffer[0] = 0x00;
	VCP_send_buffer[1] = 0x20;
	VCP_send_index = 4;
}

static void Parse_BLE_Cmd(void)
{
  u8 temp_buffer[80];
  u8 ret_buffer[80];
  u8 ret_index = 0;
  u8 length;
  u8 read_num;
  
  u32 adv_time;  
  
  Buffer_Read(&VCP_RX_Index, temp_buffer, 2);//uid not care
  if(temp_buffer[1] & 0xF0)
  {
    length = temp_buffer[1] >> 4;
  }else
  {
    Buffer_Read(&VCP_RX_Index, &length, 1);
  }  
  
  Buffer_Read(&VCP_RX_Index, temp_buffer, length);
  switch(temp_buffer[0])
  {
    case 0x08:
      ret_buffer[ret_index++] = 0x00;
      read_num = temp_buffer[1] > Get_BLE_RX_Length() ? Get_BLE_RX_Length() : temp_buffer[1];
      if(read_num > 0xD)
      {
        ret_buffer[ret_index++] = 0x00;
        ret_buffer[ret_index++] = read_num+2;        
      }
      else
      {
        ret_buffer[ret_index++] = (read_num+2) << 4;        
      }
      ret_buffer[ret_index++] = read_num;
      BLE_Read_Buffer(&ret_buffer[ret_index], read_num);    
      ret_index += read_num;
      ret_buffer[ret_index] = 0;
      for(u8 i = 0;i < ret_index;i++)
      {
        ret_buffer[ret_index] += ret_buffer[i];
      }
      ret_buffer[ret_index] = ~ret_buffer[ret_index];
      ret_index++;
    break;
      
    case 0x09:
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      ret_buffer[ret_index++] = BLE_Write_Buffer(&temp_buffer[2], temp_buffer[1]);
      ret_buffer[ret_index++] = (u8)(~(ret_buffer[0]+ret_buffer[1]+ret_buffer[2]));
    break;
    
    case 0x0A:
      temp_buffer[length-1] = 0;//覆蓋CS
      BLE_Set_Name((const char*)&temp_buffer[1]);
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0xDF;
    break;

    case 0x0B:
      #if 0
      temp_buffer[length-1] = 0;//覆蓋CS
      BLE_Set_Address(&temp_buffer[1]);
      #endif
      ret_buffer[ret_index++] = 0x01;
      ret_buffer[ret_index++] = 0x20;
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0xDE;
    break;

    case 0x0C:
      BLE_Power_Down();
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0xDF;
    break;

    case 0x0D:
      BLE_Power_Up();
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0xDF;
    break;   

    case 0x0E:
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      ret_buffer[ret_index++] = Get_BLE_Status();
      ret_buffer[ret_index++] = (u8)(~(ret_buffer[0]+ret_buffer[1]+ret_buffer[2]));
    break;
    
    case 0x0F:
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      ret_buffer[ret_index++] = !(BLE_Advertise_Data(temp_buffer[1], &temp_buffer[2]));
      ret_buffer[ret_index++] = (u8)(~(ret_buffer[0]+ret_buffer[1]+ret_buffer[2]));
    break;
    
    case 0x10:
      BLE_Set_Advertise(temp_buffer[1] | temp_buffer[2]<<8 | temp_buffer[3]<<16 | temp_buffer[4]<<24);
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0xDF;
    break;
    
    case 0x11:
      adv_time = Get_Advertise_Time();
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      ret_buffer[ret_index++] = (adv_time) & 0xFF;
      ret_buffer[ret_index++] = (adv_time >> 8) & 0xFF;
      ret_buffer[ret_index++] = (adv_time >> 16) & 0xFF;
      ret_buffer[ret_index++] = (adv_time >> 24) & 0xFF;
      for(u8 i = 0;i < ret_index;i++)
      {
        ret_buffer[ret_index] += ret_buffer[i];
      }
      ret_buffer[ret_index] = ~ret_buffer[ret_index];
      ret_index++;
    break;
      
    case 0x12:
      Stop_Advertise();
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0xDF;
    break;

    case 0x13:
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;      
      ret_buffer[ret_index++] =  Get_BLE_RX_Length();
      ret_buffer[ret_index++] = (u8)(~(ret_buffer[0]+ret_buffer[1]+ret_buffer[2]));
    break;
    
    case 0x14:
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;      
      ret_buffer[ret_index++] =  Get_BLE_TX_Length();    
      ret_buffer[ret_index++] = (u8)(~(ret_buffer[0]+ret_buffer[1]+ret_buffer[2]));
    break;

    default:
      Buffer_Discard(&VCP_RX_Index);
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      ret_buffer[ret_index++] = 0x01;
      ret_buffer[ret_index++] = 0xDE;
  }
  memcpy(VCP_send_buffer, ret_buffer, ret_index);
  VCP_send_index = ret_index;
}
/*********************************************************************************************************//**
 * @brief Parse the retval packet from module used UR_BUS, and fill VCP_send_buffer
 * @param which UR_BUS number
 * @retval Check pass or fail
 ************************************************************************************************************/
static bool Parse_Retval(u8 num)
{
  /*
  v1.0 Module-->mcu mid+eid_len+(len2)+status+retval+cs
  mcu校驗并解析為mid+eid+status+retval形式回傳上位機
  
  u8 MID[2];
  u8 Ten_EID;
  u8 Length;
  u8 Length_backup = Get_Valid_Lenth(&HS_RX_Index);
  u32 check_sum = 0;

  Buffer_Read(&HS_RX_Index, MID, 2);//mid
  if(MID[1] != 0x01)
    return FALSE;
  VCP_send_buffer[VCP_send_index] = MID[0];
  VCP_send_index++;
  check_sum += MID[0];
  
  Buffer_Read(&HS_RX_Index, &Ten_EID, 1);//EID
  if(Ten_EID & 0xF0)
  {
    Length = Ten_EID>>4;
    if(Length != Length_backup-3)
      return FALSE;
    VCP_send_buffer[VCP_send_index] = Ten_EID & 0x0F;
    VCP_send_index++;
    check_sum += Ten_EID;
  }else
  {
    Buffer_Read(&HS_RX_Index, &Length, 1);
    if(Length != Length-4)
      return FALSE;
    VCP_send_buffer[VCP_send_index] = Ten_EID;
    VCP_send_index++;   
    check_sum += Ten_EID + Length;  
  }
  Buffer_Read(&HS_RX_Index, &VCP_send_buffer[VCP_send_index], Length);

  for(u8 i = 0;i < Length;i++)
  {
    check_sum += VCP_send_buffer[VCP_send_index++];
  }
  check_sum = ~check_sum;
  check_sum &= 0xFF;
  if(check_sum != VCP_send_buffer[VCP_send_index])
    return FALSE;

  VCP_send_index--;//get rid of CS
  
  return TRUE;
  */
  
  /*
  v2.0 Module-->mcu mid+eid_len+(len2)+status+retval+cs
  mcu需要做以下工作：
  1.判斷MID最高位是否為1，如果是則回傳上位機全部內容，否則丟棄該筆數據
  2.其餘數據不做任何解析直接bypass
  3.BUS_RX_buffer數據copy至VCP_send_buffer，VCP_send_index遞增
  */
  Buffer_Read(&BUS_RX_Index[num], VCP_send_buffer, 2);
  if(VCP_send_buffer[1] != 0x01)
  {
    Buffer_Discard(&BUS_RX_Index[num]);
    return FALSE;
  }
  VCP_send_index = Get_Valid_Lenth(&BUS_RX_Index[num]);
  Buffer_Read(&BUS_RX_Index[num], &VCP_send_buffer[1], VCP_send_index);  
  VCP_send_index++;
  return TRUE;  
}

static void Parse_System_Cmd(void)
{
  u8 temp_buffer[80];
  u8 ret_buffer[80];
  u8 ret_index = 0;
  u8 length;
  
  Buffer_Read(&VCP_RX_Index, temp_buffer, 2);//uid not care
  if(temp_buffer[1] & 0xF0)
  {
    length = temp_buffer[1] >> 4;
  }else
  {
    Buffer_Read(&VCP_RX_Index, &length, 1);
  }  
  
  Buffer_Read(&VCP_RX_Index, temp_buffer, length);
  switch(temp_buffer[0])
  {  
    case 0x00:
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      if(Standby_flag == TRUE)
      {
        ret_buffer[ret_index++] = 0x01;
        ret_buffer[ret_index++] = 0xDE;
      }
      else
      {
        Bus_SystemResetAll();
        ret_buffer[ret_index++] = 0x00;
        ret_buffer[ret_index++] = 0xDF;
      }
      break;
      
    case 0x01:
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      if(Standby_flag == TRUE)
      {
        ret_buffer[ret_index++] = 0x01;
        ret_buffer[ret_index++] = 0xDE;
      }
      else
      {
        Bus_SystemStandby();
        Bus_DeleteUartFunc();        
        ret_buffer[ret_index++] = 0x00;
        ret_buffer[ret_index++] = 0xDF;
      }
      break;
      
    case 0x02:
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      if(Standby_flag == TRUE)
      {
        ret_buffer[ret_index++] = 0x01;
        ret_buffer[ret_index++] = 0xDE;
      }
      else
      {
        Bus_SystemAction();
        ret_buffer[ret_index++] = 0x00;
        ret_buffer[ret_index++] = 0xDF;
      }
      break;
      
    case 0x03:
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      if(Standby_flag == FALSE)
      {
        ret_buffer[ret_index++] = 0x01;
        ret_buffer[ret_index++] = 0xDE;
      }
      else
      {
        Wakeup_AllBUS();
        ret_buffer[ret_index++] = 0x00;
        ret_buffer[ret_index++] = 0xDF;
      }
      break;
      
    case 0x04:
      ret_buffer[ret_index++] = 0x00;
      ret_buffer[ret_index++] = 0x20;
      if(ReadWUS_stage == 1 || ReadWUS_stage == 2)
      {
        ret_buffer[ret_index++] = 0x00;
        ret_buffer[ret_index++] = Wakeup_record;
        ret_buffer[ret_index++] = ~(0x20+Wakeup_record);
        if(ReadWUS_stage == 2)
        {
          ReadWUS_stage = 0;
          Wakeup_record = 0;
        }
      }
      else
      {
        ret_buffer[ret_index++] = 0x01;
        ret_buffer[ret_index++] = 0xDE;         
      }
    default:
      break;
  }
  memcpy(VCP_send_buffer, ret_buffer, ret_index);
  VCP_send_index = ret_index;
}

/*********************************************************************************************************//**
 * @brief Wait for UR_BUS receive packet from module
 * @param None
 * @retval Receive status
 ************************************************************************************************************/
RX_STATUS_ENUM UR_Bus_Receive(u8 num)
{
  static u8 stage[3] = {0};
  static u16 receive_num[3];
  u16 len = 0;
  
  switch(stage[num])
  {
    case 0://start to receive packet
      Bus_Set_Timeout(num, BUS_WAITING_TIMEOUT);
      USART_RxCmd(BUS_Port_table[num], ENABLE);//enable receive function
      stage[num]++;
      busRXCount[num] = 0;
      return WAITING;

    case 1://wait for the first data
      if(Get_Valid_Lenth(&BUS_RX_Index[num]))
      {
        Bus_Set_Timeout(num, BUS_INTERVAL_TIMEOUT);
        receive_num[num] = Get_Valid_Lenth(&BUS_RX_Index[num]);
        stage[num]++;
        return RECEIVING;
      }
      else if(Check_Bus_Timeout(num))
      {
        USART_RxCmd(BUS_Port_table[num], DISABLE);//disable receive function
        stage[num] = 0;
        return NO_RESPONSE;//timeout no response
      }
      else return WAITING; 

    case 2://wait for timeout to indicate receive complete
      //initialize for expect length
      if(busRXCount[num] == 0 && receive_num[num] >= 3)
      {
        Buffer_ReadByteExt(&BUS_RX_Index[num], (u8*)&len, 2);
        if(len & 0xF0)
        {
          busRXCount[num] = (len >> 4) + 2 + 1;
        }
      }
      if(busRXCount[num] == 0 && receive_num[num] >= 4)
      {
        Buffer_ReadByteExt(&BUS_RX_Index[num], (u8*)&len, 3);
        busRXCount[num] = len + 2 + 1;
      }
      //check for new data or timeout      
      if(Get_Valid_Lenth(&BUS_RX_Index[num]) > receive_num[num])//data update
      {
        receive_num[num] = Get_Valid_Lenth(&BUS_RX_Index[num]);
        Bus_Set_Timeout(num, BUS_INTERVAL_TIMEOUT);
        return RECEIVING;        
      }
      else if(Check_Bus_Timeout(num) || receive_num[num] == busRXCount[num])
      {
        //data receive complete
        USART_RxCmd(BUS_Port_table[num], DISABLE);//disable receive function
        stage[num]++;
        return VCP_BUSY;//other urbus is using VCP to parse retval,wait for idle 
      }
      else return INTERVAL; 
      
    case 3://parse retval packet
      if(VCP_send_index)
        return VCP_BUSY;
      stage[num] = 0;
      if(Parse_Retval(num))
        return RX_OK;
      else
        return CHECK_ERR;
  }
  return CHECK_ERR;
}

/*********************************************************************************************************//**
 * @brief Mian loop function handler
 * @param None
 * @retval None
 ************************************************************************************************************/
void Handler(void)
{
  RX_STATUS_ENUM status;
  u8 cmd;
  
  if(VCP_RX_finish)
  {
    VCP_RX_finish = FALSE;
    
    Buffer_Read(&VCP_RX_Index, &cmd, 1);//CMD code
    if(cmd == 0x55)//IAP cmd
    {
      extern u32 u32BufferIndex;
      extern u8 gu8CmdBuffer[79];
      
      u32BufferIndex = 0;
      gu8CmdBuffer[u32BufferIndex++] = 0x55;
      u16 len_temp = Get_Valid_Lenth(&VCP_RX_Index);
      Buffer_Read(&VCP_RX_Index, &gu8CmdBuffer[1], len_temp);
      u32BufferIndex += len_temp;
      ParseCmd();
      return;
    }
    
    switch(cmd & 0x7F)//not iap cmd, but application cmd
    {
      case 0x01:
        #if Printf_Debug >= 2
        printf("LST:");
        printf("0x%03X ", Bus_send_buffer[0] | 0x100);
        for(u8 i = 1;i < Bus_send_index;i++)
        printf("0x%03X ", Bus_send_buffer[i]);
        printf("\r\n");
        #endif
        Parse_Bus_Cmd();
        if(Standby_flag == TRUE)
        {
          Bus_send_index = 0;
          return;
        }      
        Uart_Send_Data(LS_BUS_NUM, Bus_send_buffer[0] | 0x100);
        Uart_Send_Buffer(LS_BUS_NUM, &Bus_send_buffer[1], Bus_send_index-1);
        Bus_send_index = 0; 
        if(cmd & 0x80)
        {
          UR_Bus_response_flag |= 0x1 << LS_BUS_NUM;
        }
      break;
      case 0x02:
        #if Printf_Debug >= 2
        printf("HST:");
        printf("0x%03X ", Bus_send_buffer[0] | 0x100);
        for(u8 i = 1;i < Bus_send_index;i++)
        printf("0x%03X ", Bus_send_buffer[i]);
        printf("\r\n");
        #endif
        Parse_Bus_Cmd();
        if(USR0_using_flag != USART0_IDLE)
        {
          Bus_send_index = 0;
          return;
        }
        if(Standby_flag == TRUE)
        {
          Bus_send_index = 0;
          return;
        }
        //USART0 used by HS
        USR0_using_flag = USART0_HS_USE;
        AFIO_GPxConfig(HV_BUS_TX_GPIO_ID, HV_BUS_TX_AFIO_PIN, AFIO_FUN_DEFAULT);
        AFIO_GPxConfig(HV_BUS_RX_GPIO_ID, HV_BUS_RX_AFIO_PIN, AFIO_FUN_DEFAULT);         
        AFIO_GPxConfig(HS_BUS_TX_GPIO_ID, HS_BUS_TX_AFIO_PIN, HS_BUS_TX_AFIO_MODE);
        AFIO_GPxConfig(HS_BUS_RX_GPIO_ID, HS_BUS_RX_AFIO_PIN, HS_BUS_RX_AFIO_MODE);
        
        Uart_Send_Data(HS_BUS_NUM, Bus_send_buffer[0] | 0x100);
        Uart_Send_Buffer(HS_BUS_NUM, &Bus_send_buffer[1], Bus_send_index-1);
        Bus_send_index = 0; 
        if(cmd & 0x80)
        {
          UR_Bus_response_flag |= 0x1 << HS_BUS_NUM;
        }
        else
        {
          USR0_using_flag = USART0_IDLE;
        }
      break;        
      case 0x04:
        #if Printf_Debug >= 2
        printf("HVT:");
        printf("0x%03X ", Bus_send_buffer[0] | 0x100);
        for(u8 i = 1;i < Bus_send_index;i++)
        printf("0x%03X ", Bus_send_buffer[i]);
        printf("\r\n");
        #endif
        Parse_Bus_Cmd();
        if(USR0_using_flag != USART0_IDLE)
        {
          Bus_send_index = 0;
          return;
        }
        if(Standby_flag == TRUE)
        {
          Bus_send_index = 0;
          return;
        }        
        //USART0 used by HV
        USR0_using_flag = USART0_HV_USE;
        AFIO_GPxConfig(HS_BUS_TX_GPIO_ID, HS_BUS_TX_AFIO_PIN, AFIO_FUN_DEFAULT);
        AFIO_GPxConfig(HS_BUS_RX_GPIO_ID, HS_BUS_RX_AFIO_PIN, AFIO_FUN_DEFAULT);         
        AFIO_GPxConfig(HV_BUS_TX_GPIO_ID, HV_BUS_TX_AFIO_PIN, HV_BUS_TX_AFIO_MODE);
        AFIO_GPxConfig(HV_BUS_RX_GPIO_ID, HV_BUS_RX_AFIO_PIN, HV_BUS_RX_AFIO_MODE);
        
        Uart_Send_Data(HV_BUS_NUM, Bus_send_buffer[0] | 0x100);
        Uart_Send_Buffer(HV_BUS_NUM, &Bus_send_buffer[1], Bus_send_index-1);
        Bus_send_index = 0; 
        if(cmd & 0x80)
        {
          UR_Bus_response_flag |= 0x1 << HV_BUS_NUM;
        }
        else
        {
          USR0_using_flag = USART0_IDLE;
        }
      break;
      case 0x08:
        Parse_Voice_Cmd();
        #if Printf_Debug >= 1
        printf("Audio excute result:");
        if(VCP_send_buffer[2] != 0x0A)
        {
          for(u8 i = 0;i < VCP_send_index;i++)
          {
            printf("%02X ",VCP_send_buffer[i]);
          }
          printf("\r\n");
        }
        else printf("Audio command error!\r\n");
        #else
        if(cmd & 0x80)
        {
          for(u8 i = 0;i < VCP_send_index;i++)
          SERIAL_PutChar(VCP_send_buffer[i]);
        }
        SERIAL_Flush();
        #endif
        VCP_send_index = 0;
      break;
      case 0x10:
        Parse_BLE_Cmd();
        if(cmd & 0x80)
        {
          for(u8 i = 0;i < VCP_send_index;i++)
          SERIAL_PutChar(VCP_send_buffer[i]);
        }
        SERIAL_Flush();
        VCP_send_index = 0;  
      break;

      case 0x20:
        Parse_System_Cmd();
        for(u8 i = 0;i < VCP_send_index;i++)
        SERIAL_PutChar(VCP_send_buffer[i]);
        SERIAL_Flush();
        VCP_send_index = 0;
      
      default:
        Buffer_Discard(&VCP_RX_Index);
    }
  }
  if(UR_Bus_response_flag & (0x1 << HS_BUS_NUM))
  {
    status = UR_Bus_Receive(HS_BUS_NUM);
    switch(status)
    {
      case WAITING:
      case INTERVAL:
      case RECEIVING:
      case VCP_BUSY:
      break;
      case NO_RESPONSE:
        UR_Bus_response_flag &= ~(0x1 << HS_BUS_NUM);
        USR0_using_flag = USART0_IDLE;
        #if Printf_Debug >= 1
        printf("HS_BUS Module no response\r\n");
        #endif
      break;
      case RX_OK:
        UR_Bus_response_flag &= ~(0x1 << HS_BUS_NUM);
        USR0_using_flag = USART0_IDLE;      
        #if Printf_Debug >= 1
        printf("HS_BUS Receive succeed!!!\r\n");
        printf("Receive data:");
        for(u8 i = 0;i < VCP_send_index;i++)
        {
          printf("%02X ",VCP_send_buffer[i]);
        }
        printf("\r\n");
        #else
        for(u8 i = 0;i < VCP_send_index;i++)
          SERIAL_PutChar(VCP_send_buffer[i]);
        SERIAL_Flush();
        #endif
        VCP_send_index = 0;
      break;
      case CHECK_ERR:
        UR_Bus_response_flag &= ~(0x1 << HS_BUS_NUM);
        USR0_using_flag = USART0_IDLE;
        #if Printf_Debug >= 1
        printf("HS_BUS Receive Finish,but check error\r\n");
        #endif
    }
  }
  if(UR_Bus_response_flag & (0x1 << LS_BUS_NUM))
  {
    status = UR_Bus_Receive(LS_BUS_NUM);
    switch(status)
    {
      case WAITING:
      case INTERVAL:
      case RECEIVING:
      case VCP_BUSY:
      break;
      case NO_RESPONSE:
        UR_Bus_response_flag &= ~(0x1 << LS_BUS_NUM);
        #if Printf_Debug >= 1
        printf("LS_BUS Module no response\r\n");
        #endif
      break;
      case RX_OK:
        UR_Bus_response_flag &= ~(0x1 << LS_BUS_NUM);
        #if Printf_Debug >= 1
        printf("LS_BUS Receive succeed!!!\r\n");
        printf("Receive data:");
        for(u8 i = 0;i < VCP_send_index;i++)
        {
          printf("%02X ",VCP_send_buffer[i]);
        }
        printf("\r\n");
        #else
        for(u8 i = 0;i < VCP_send_index;i++)
          SERIAL_PutChar(VCP_send_buffer[i]);
        SERIAL_Flush();
        #endif
        VCP_send_index = 0;
      break;
      case CHECK_ERR:
        UR_Bus_response_flag &= ~(0x1 << LS_BUS_NUM);
        #if Printf_Debug >= 1
        printf("LS_BUS Receive Finish,but check error\r\n");
        #endif
    }
  }
  if(UR_Bus_response_flag & (0x1 << HV_BUS_NUM))
  {
    status = UR_Bus_Receive(HV_BUS_NUM);
    switch(status)
    {
      case WAITING:
      case INTERVAL:
      case RECEIVING:
      case VCP_BUSY:
      break;
      case NO_RESPONSE:
        UR_Bus_response_flag &= ~(0x1 << HV_BUS_NUM);
        USR0_using_flag = USART0_IDLE;
        #if Printf_Debug >= 1
        printf("HV_BUS Module no response\r\n");
        #endif
      break;
      case RX_OK:
        UR_Bus_response_flag &= ~(0x1 << HV_BUS_NUM);
        USR0_using_flag = USART0_IDLE;
        #if Printf_Debug >= 1
        printf("HV_BUS Receive succeed!!!\r\n");
        printf("Receive data:");
        for(u8 i = 0;i < VCP_send_index;i++)
        {
          printf("%02X ",VCP_send_buffer[i]);
        }
        printf("\r\n");
        #else
        for(u8 i = 0;i < VCP_send_index;i++)
          SERIAL_PutChar(VCP_send_buffer[i]);
        SERIAL_Flush();
        #endif
        VCP_send_index = 0;
      break;
      case CHECK_ERR:
        UR_Bus_response_flag &= ~(0x1 << HV_BUS_NUM);
        USR0_using_flag = USART0_IDLE;
        #if Printf_Debug >= 1
        printf("HV_BUS Receive Finish,but check error\r\n");
        #endif
    }
  }
}

/*********************************************************************************************************//**
 * @brief   This function handles HS_BUS & HV_BUS interrupt.
 * @retval  None
 ************************************************************************************************************/
void HS_BUS_IRQHandler(void)//HV_BUS_IRQHandler共用
{
  u16 temp;
  if(USART_GetFlagStatus(HS_BUS_PORT, USART_FLAG_RXDR))
  {
    temp = USART_ReceiveData(HS_BUS_PORT);
    if(USR0_using_flag == USART0_HS_USE)
    {
      if(temp & 0x100)
        Buffer_Write(&BUS_RX_Index[HS_BUS_NUM], (u8*)&temp, 2);
      else
        Buffer_Write(&BUS_RX_Index[HS_BUS_NUM], (u8*)&temp, 1);
    }
    else if(USR0_using_flag == USART0_HV_USE)
    {
      if(temp & 0x100)
        Buffer_Write(&BUS_RX_Index[HV_BUS_NUM], (u8*)&temp, 2);
      else
        Buffer_Write(&BUS_RX_Index[HV_BUS_NUM], (u8*)&temp, 1);      
    }
  }
}
/*********************************************************************************************************//**
 * @brief   This function handles LS_BUS interrupt.
 * @retval  None
 ************************************************************************************************************/
void LS_BUS_IRQHandler(void)
{
  u16 temp;
  if(USART_GetFlagStatus(LS_BUS_PORT, USART_FLAG_RXDR))
  {
    temp = USART_ReceiveData(LS_BUS_PORT);
    if(temp & 0x100)
      Buffer_Write(&BUS_RX_Index[LS_BUS_NUM], (u8*)&temp, 2);
    else
      Buffer_Write(&BUS_RX_Index[LS_BUS_NUM], (u8*)&temp, 1);
  }
}

/*********************************************************************************************************//**
 * @brief UART_BUS Transmit function
 * @param bus LS_BUS_NUM; HS_BUS_NUM; HV_BUS_NUM
 * @param uid = MID << 4 | EID
 * @param len = length of struction and paramenter
 * @param buffer of put struction and paramenter
 * @retval all count of send byte succeed(MID+Tlen&EID+/Tlen2/+Instrution+Para+CS)
 ************************************************************************************************************/
static u32 Bus_Transmit(u8 bus, u16 uid, u8 len, u8* par)
{
  u32 ret_num = 0;
  u32 check_sum = 0;
  u8 Tlen[2];
  //MID
  Uart_Send_Data(bus, (uid >> 4) | 0x100);
  check_sum += uid >> 4;
  ret_num++;  
  //EID and length
  if(len+1 > 0xF)
  {
    Tlen[0] = uid & 0x0F;
    Tlen[1] = len+1;
    ret_num += 2;
    check_sum += Tlen[0]+Tlen[1];
    Uart_Send_Buffer(bus, Tlen, 2);
  }
  else
  {
    Tlen[0] = ((len+1) << 4) | (uid & 0x0F);
    ret_num++;
    check_sum += Tlen[0];
    Uart_Send_Buffer(bus, Tlen, 1);
  }  
  //struction and paramenter
  ret_num += len;
  for(u16 i = 0;i < len;i++)
  {
    check_sum += par[i];
  }
  Uart_Send_Buffer(bus, par, len);
  //check_sum
  check_sum = ~check_sum;
  check_sum = check_sum & 0xFF;
  Uart_Send_Buffer(bus, (u8*)&check_sum, 1);
  ret_num++;
  
  return ret_num;  
}

/*********************************************************************************************************//**
 * @brief UART_BUS Transmit API interface
 * @param uid = MID << 4 | EID
 * @param len = length of struction and paramenter
 * @param buffer of put struction and paramenter
 * @retval all count of send byte succeed(MID+Tlen&EID+/Tlen2/+Instrution+Para+CS)
 ************************************************************************************************************/
int LS_Transmit(u16 uid, u8 len, u8 *par)
{
  return Bus_Transmit(LS_BUS_NUM, uid, len, par);
}

int HS_Transmit(u16 uid, u8 len, u8 *par)
{
  AFIO_GPxConfig(HV_BUS_TX_GPIO_ID, HV_BUS_TX_AFIO_PIN, AFIO_FUN_DEFAULT);
  AFIO_GPxConfig(HV_BUS_RX_GPIO_ID, HV_BUS_RX_AFIO_PIN, AFIO_FUN_DEFAULT);         
  AFIO_GPxConfig(HS_BUS_TX_GPIO_ID, HS_BUS_TX_AFIO_PIN, HS_BUS_TX_AFIO_MODE);
  AFIO_GPxConfig(HS_BUS_RX_GPIO_ID, HS_BUS_RX_AFIO_PIN, HS_BUS_RX_AFIO_MODE);
  return Bus_Transmit(HS_BUS_NUM, uid, len, par);
}

int HV_Transmit(u16 uid, u8 len, u8 *par)
{
  AFIO_GPxConfig(HS_BUS_TX_GPIO_ID, HS_BUS_TX_AFIO_PIN, AFIO_FUN_DEFAULT);
  AFIO_GPxConfig(HS_BUS_RX_GPIO_ID, HS_BUS_RX_AFIO_PIN, AFIO_FUN_DEFAULT);         
  AFIO_GPxConfig(HV_BUS_TX_GPIO_ID, HV_BUS_TX_AFIO_PIN, HV_BUS_TX_AFIO_MODE);
  AFIO_GPxConfig(HV_BUS_RX_GPIO_ID, HV_BUS_RX_AFIO_PIN, HV_BUS_RX_AFIO_MODE);  
  return Bus_Transmit(HV_BUS_NUM, uid, len, par);
}

/*********************************************************************************************************//**
 * @brief Parse the retval packet from module used UR_BUS, and fill VCP_send_buffer
 * @param which UR_BUS number
 * @retval Check pass or fail
 ************************************************************************************************************/
static bool Parse_Retval_Ext(u8 bus, u16* uid, u8* len, u8* par)
{
  u8 MID[2];
  u8 Ten_EID;
  u8 Length;
  u8 Length_backup = Get_Valid_Lenth(&BUS_RX_Index[bus]);
  u32 check_sum = 0;
  u8 CS_backup;
  
  Buffer_Read(&BUS_RX_Index[bus], MID, 2);//mid
  if(MID[1] != 0x01)
    return FALSE;
  *uid = (u16)MID[0] << 4;
  check_sum += MID[0];

  Buffer_Read(&BUS_RX_Index[bus], &Ten_EID, 1);//EID
  if(Ten_EID & 0xF0)
  {
    Length = Ten_EID >> 4;
    if(Length != Length_backup-3)
      return FALSE;
    *uid |= Ten_EID & 0xF;
    check_sum += Ten_EID;
  }else
  {
    Buffer_Read(&BUS_RX_Index[bus], &Length, 1);
    if(Length != Length_backup-4)
      return FALSE;
    *uid |= Ten_EID;  
    check_sum += Ten_EID + Length;  
  }
  Length--;//cs
  Buffer_Read(&BUS_RX_Index[bus], par, Length);//read instru+para  
  for(u8 i = 0;i < Length;i++)
  {
    check_sum += par[i];
  }
  Buffer_Read(&BUS_RX_Index[bus], &CS_backup, 1);//read cs  
  
  check_sum = ~check_sum;
  check_sum &= 0xFF;
  if(check_sum != CS_backup)
    return FALSE;
  
  *len = Length;
  return TRUE;
}

/*********************************************************************************************************//**
 * @brief Wait for UR_BUS receive packet from module
 * @param None
 * @retval Receive status
 ************************************************************************************************************/
static RX_STATUS_ENUM UR_Bus_Receive_Ext(u8 bus, u16* uid, u8* len, u8* par)
{
  static u8 stage[3] = {0};
  static u16 receive_num[3];
  u16 len_temp = 0; 
  
  switch(stage[bus])
  {
    case 0://start to receive packet
      *len = 0;
      Bus_Set_Timeout(bus, BUS_WAITING_TIMEOUT);
      USART_RxCmd(BUS_Port_table[bus], ENABLE);//enable receive function
      stage[bus]++;
      busRXCount[bus] = 0;
      return WAITING;

    case 1://wait for the first data
      if(Get_Valid_Lenth(&BUS_RX_Index[bus]))
      {
        Bus_Set_Timeout(bus, BUS_INTERVAL_TIMEOUT);
        receive_num[bus] = Get_Valid_Lenth(&BUS_RX_Index[bus]);
        stage[bus]++;
        return RECEIVING;
      }
      else if(Check_Bus_Timeout(bus))
      {
        USART_RxCmd(BUS_Port_table[bus], DISABLE);//disable receive function
        stage[bus] = 0;
        return NO_RESPONSE;//timeout no response
      }
      else return WAITING; 

    case 2://wait for timeout to indicate receive complete
      if(busRXCount[bus] == 0 && receive_num[bus] >= 3)
      {
        Buffer_ReadByteExt(&BUS_RX_Index[bus], (u8*)&len_temp, 2);
        if(len_temp & 0xF0)
        {
          busRXCount[bus] = (len_temp >> 4) + 2 + 1;
        }
      }
      if(busRXCount[bus] == 0 && receive_num[bus] >= 4)
      {
        Buffer_ReadByteExt(&BUS_RX_Index[bus], (u8*)&len_temp, 3);
        busRXCount[bus] = len_temp + 2 + 1;
      }
      
      if(Get_Valid_Lenth(&BUS_RX_Index[bus]) > receive_num[bus])//data update
      {
        receive_num[bus] = Get_Valid_Lenth(&BUS_RX_Index[bus]);
        Bus_Set_Timeout(bus, BUS_INTERVAL_TIMEOUT);
        return RECEIVING;        
      }
      else if(Check_Bus_Timeout(bus) || receive_num[bus] == busRXCount[bus])
      {
        //data receive complete
        USART_RxCmd(BUS_Port_table[bus], DISABLE);//disable receive function
        stage[bus] = 0;
        if(Parse_Retval_Ext(bus, uid, len, par))
         return RX_OK; 
        else
        {
          Buffer_Discard(&BUS_RX_Index[bus]);
          return CHECK_ERR;
        }
      }
      else return INTERVAL;     
  }
  return CHECK_ERR;
}

/*********************************************************************************************************//**
 * @brief UR_BUS receive API interface
 * @param None
 * @retval Receive status
 * @note The retval 0--OK,1--No Res,2--Check Error means the end of receive process, others should not care.
          Only when retval is 0--OK, paramenter is valid.
 ************************************************************************************************************/
int LS_Receive(u16* uid, u8* len, u8* par)
{
  RX_STATUS_ENUM ret;
  do{
    ret = UR_Bus_Receive_Ext(LS_BUS_NUM, uid, len, par);
    Handler();
    MIDI_Process();
    BLE_process();
    if(HT_CKCU->APBCCR1 & (1 << 4))
      WDT_Restart();
  }while(ret > CHECK_ERR);
  return ret;
}

int HS_Receive(u16* uid, u8* len, u8* par)
{
  RX_STATUS_ENUM ret;
  do{
    ret = UR_Bus_Receive_Ext(HS_BUS_NUM, uid, len, par);
    Handler();
    MIDI_Process();
    BLE_process();
    if(HT_CKCU->APBCCR1 & (1 << 4))
      WDT_Restart();    
  }while(ret > CHECK_ERR);
  USR0_using_flag = USART0_IDLE;
  return ret;
}

int HV_Receive(u16* uid, u8* len, u8* par)
{
  RX_STATUS_ENUM ret;
  do{
    ret = UR_Bus_Receive_Ext(HV_BUS_NUM, uid, len, par);
    Handler();
    MIDI_Process();
    BLE_process();
    if(HT_CKCU->APBCCR1 & (1 << 4))
      WDT_Restart();    
  }while(ret > CHECK_ERR);
  USR0_using_flag = USART0_IDLE;
  return ret;
}

void BFTM1_IRQHandler(void)
{
  BFTM_ClearFlag(HT_BFTM1);
  HT_GPIOC->DOUTR ^= 0x1 << 3;
  //uart bus timer related
  if(Bus_Timeout_Value[LS_BUS_NUM]) Bus_Timeout_Value[LS_BUS_NUM]--;
  if(Bus_Timeout_Value[HS_BUS_NUM]) Bus_Timeout_Value[HS_BUS_NUM]--;
  if(Bus_Timeout_Value[HV_BUS_NUM]) Bus_Timeout_Value[HV_BUS_NUM]--;
  /***************BLE & APP related***************/
  extern vu32 BLE_APPConnect;
  if(BLE_APPConnect) BLE_APPConnect--;  
  //BLE timer related
  static u16 tm_cnt = 0;
  if(BLE_delay) BLE_delay--;
  if(++tm_cnt == 1000/BFTM_TIME_BASE) //1ms
  {
    tm_cnt = 0;
    timer_operate.bits.t1ms = TRUE;
  }
}
