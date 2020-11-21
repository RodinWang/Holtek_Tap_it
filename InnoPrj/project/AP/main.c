 /************************************************************************************************************
 * @attention
 *
 * Firmware Disclaimer Information
 *
 * 1. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, which is supplied by Holtek Semiconductor Inc., (hereinafter referred to as "HOLTEK") is the
 *    proprietary and confidential intellectual property of HOLTEK, and is protected by copyright law and
 *    other intellectual property laws.
 *
 * 2. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, is confidential information belonging to HOLTEK, and must not be disclosed to any third parties
 *    other than HOLTEK and the customer.
 *
 * 3. The program technical documentation, including the code, is provided "as is" and for customer reference
 *    only. After delivery by HOLTEK, the customer shall use the program technical documentation, including
 *    the code, at their own risk. HOLTEK disclaims any expressed, implied or statutory warranties, including
 *    the warranties of merchantability, satisfactory quality and fitness for a particular purpose.
 *
 * <h2><center>Copyright (C) Holtek Semiconductor Inc. All rights reserved</center></h2>
 ************************************************************************************************************/
//-----------------------------------------------------------------------------
#include "ht32.h"
#include "urbus.h"
#include "iap_handler.h"
#include "user.h"
#include "GPIO.h"
#include "sys_timer.h"
#include "bleuser.h"
#include "bleApp.h"
#include "Power.h"
#include "MPU6050.h"
// my const file
#include "MyConst.h"
#include "SongMap.h"
#include "stdlib.h"
//-----------------------------------------------------------------------------
extern u16 IAP_UART_Send(u8 *puData, u16 length);
// common variable
u8 len;
u8 cmd[256];
u8 buf[256];
u16 id;
u8 ret;
ENUM_RESTYPE ble_ret;
bool bResult;

// Const LED EID
const u8 EID_LED[] = { EID_LED_1, EID_LED_2, EID_LED_3, EID_LED_4, EID_LED_5, EID_LED_6 };

// Const Vibrate detection EID
const u8 EID_VIBR[] = { EID_VIBR_1, EID_VIBR_2, EID_VIBR_3, EID_VIBR_4 };

// const GPIO Port
HT_GPIO_TypeDef *FGPIO_Port[] = { GPIO_PORT_FLEFT, GPIO_PORT_FRIGHT };
HT_GPIO_TypeDef *SWGPIO_Port[] = { GPIO_PORT_MODE_SW, GPIO_PORT_MODE_SW };

// const GPIO Pin
const u16 FGPIO_Pin[] = { GPIO_PIN_FLEFT, GPIO_PIN_FRIGHT };
const u16 GPIO_Pin_SW[] = { GPIO_PIN_MODE_SW_1, GPIO_PIN_MODE_SW_2 };

// global variable
extern vu16 RandCnt;
vu32 BeatCounter;
u8 GameMode = 0;
u8 MapCnt = 0;
vu8 Volume = 10;
u8 Temp;

u8 BT_Connected = 0;
u8 SongMode_Setup = 0;
u8 AudioCnt = 0;

u32 *pSongBeatMapTiming;

// Queue
QueueElement Queue[QUEUE_MAX];
	
// vibrate variable
u8 Vibrate_Status_prv[4] = { RESET, RESET, RESET, RESET };
u8 Vibrate_Status_now[4] = { RESET, RESET, RESET, RESET };

// foot GPIO variable
u8 FGPIO_Status_prv[2] = { SET, SET };
u8 FGPIO_Status_now[2] = { SET, SET };

// custom system reset
void System_Reset()
{
	sprintf((char*)cmd, "%c", 0x00);
	LS_Transmit(0x000, 1, cmd);
	HS_Transmit(0x000, 1, cmd);
	HV_Transmit(0x000, 1, cmd);
}


// LED multicolor rotation
u8 LED_Multi_Rotate(u8 EID, u8 HoldTime, u8 RotateMode)
{
	// led rotate
	sprintf((char*)cmd, "%c%c%c%c%c", 0x10,COLOR_BRIGHTNESS,HoldTime,RotateMode,0x00);
	HS_Transmit(0x110 | EID, 5, cmd);
	ret = HS_Receive(&id, &len, buf);	
	return ret;
}

// LED single color on
u8 LED_Single_Set(u8 EID, u8 red, u8 green, u8 blue)
{
	sprintf((char*)cmd, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 0x12,0x10,COLOR_BRIGHTNESS,0x00, COLOR_CODE_GEN(red, green, blue));
	HS_Transmit(0x110 | EID, 52, cmd);
	ret = HS_Receive(&id, &len, buf);
	return ret;
}

// beat LED animation
void BeatLEDControl(u8 EID, u8 direction)
{
	// Color for each LED panel
	u8 Color[6][3] = {{COLOR_CODE_RED},
										{COLOR_CODE_ORANGE},
										{COLOR_CODE_LIGHTGREEN}, 
										{COLOR_CODE_GREEN},
                    {COLOR_CODE_BLUE},
										{COLOR_CODE_PURPLE}};
	
	// generate CMD code and Transmit
	if (direction == LED_DIRECTION_CLOSE)
	{
		sprintf((char*)cmd, "%c%c%c%c%c%c%c%c%c%c%c%c%c", 0x09,COLOR_BRIGHTNESS,LED_ANIMATION_TIMING,0x00,0x00,0x00,Color[EID-1][0],Color[EID-1][1],Color[EID-1][2],0x01,0x00,0x00,0x10);
	}
	else
	{
		sprintf((char*)cmd, "%c%c%c%c%c%c%c%c%c%c%c%c%c", 0x09,COLOR_BRIGHTNESS,LED_ANIMATION_TIMING,Color[EID-1][0],Color[EID-1][1],Color[EID-1][2],0x00,0x00,0x00,0x01,0x00,0x00,0x10);
	}
	
	HS_Transmit(0x110 | EID, 13, cmd);
	ret = HS_Receive(&id, &len, buf);
}

// Clear Single ring LED to Black
void ClearSingleLED(u8 EID)
{
	sprintf((char*)cmd, "%c", 0x14);
	HS_Transmit(0x110 | EID, 1, cmd);
	ret = HS_Receive(&id, &len, buf);
}

// Clear all ring LED to Black
void ClearAllLED()
{
	u8 i;
	for (i = 0; i < MAX_LED_CNT; i++)
	{
		sprintf((char*)cmd, "%c", 0x14);
		HS_Transmit(0x110 | EID_LED[i], 1, cmd);
		ret = HS_Receive(&id, &len, buf);
	}
	
}

// Led test function
void LedTest(u8 VibrateCode, u8 GPIOCode)
{
	uint16_t i;
	static uint8_t LED_Cnt[6] = {10, 10, 10, 10, 10, 10};
	
	// vibrate
	if (VibrateCode & 0x01)
	{
		LED_Cnt[0] = (LED_Cnt[0] + 1) % 10;
	}
	if (VibrateCode & 0x02 )
	{
		LED_Cnt[1] = (LED_Cnt[1] + 1) % 10;
	}  
	if (VibrateCode & 0x04)
	{
		LED_Cnt[2] = (LED_Cnt[2] + 1) % 10;
	}
	if (VibrateCode & 0x08)
	{
		LED_Cnt[3] = (LED_Cnt[3] + 1) % 10;
	}
	
	// GPIO
	if (GPIOCode & 0x01)
	{
		LED_Cnt[4] = (LED_Cnt[4] + 1) % 10;
	}
	if (GPIOCode & 0x02)
	{
		LED_Cnt[5] =  (LED_Cnt[5] + 1) % 10;
	}
	
	// LED display
	for (i = 0; i < 6; i++){
		switch(LED_Cnt[i]){
			case 0:
				LED_Single_Set(EID_LED[i], COLOR_CODE_DARKRED);
			break;
			case 1:
				LED_Single_Set(EID_LED[i], COLOR_CODE_RED);
			break;
			case 2:
				LED_Single_Set(EID_LED[i], COLOR_CODE_ORANGE);
			break;
			case 3:
				LED_Single_Set(EID_LED[i], COLOR_CODE_YELLOW);
			break;
			case 4:
				LED_Single_Set(EID_LED[i], COLOR_CODE_LIGHTGREEN);
			break;
			case 5:
				LED_Single_Set(EID_LED[i], COLOR_CODE_GREEN);
			break;
			case 6:
				LED_Single_Set(EID_LED[i], COLOR_CODE_LIGHTBLUE);
			break;
			case 7:
				LED_Single_Set(EID_LED[i], COLOR_CODE_BLUE);
			break;
			case 8:
				LED_Single_Set(EID_LED[i], COLOR_CODE_PURPLE);
			break;
			case 9:
				LED_Single_Set(EID_LED[i], COLOR_CODE_BLACK);
		}
	}
	
}

// set Vibrate sensitivity
u8 Vibrate_Sensitivity_Set(u8 EID, u8 value)
{
	sprintf((char*)cmd, "%c%c", 0x08,value);
	LS_Transmit(0x210  | EID, 2, cmd);
	ret = LS_Receive(&id, &len, buf);
	return ret;
}

// obtain Vibrate info
u8 Vibrate_Detect(u8 EID)
{
	// vibrate detection
	sprintf((char*)cmd, "%c", 0x0A);
	LS_Transmit(0x210 | EID, 1, cmd);
	ret = LS_Receive(&id, &len, buf);
	if (ret)
	{
		return RESET;
	}
	else
	{
		return (buf[0] == 0x01);
	}
	
}

// Mode Switch GPIO Setup
void ModeGPIOSetup()
{
	GPIO_PullResistorConfig(GPIO_PORT_MODE_SW, GPIO_PIN_MODE_SW_1, GPIO_PR_UP);
	GPIO_DirectionConfig(GPIO_PORT_MODE_SW, GPIO_PIN_MODE_SW_1, GPIO_DIR_IN);
	GPIO_InputConfig(GPIO_PORT_MODE_SW, GPIO_PIN_MODE_SW_1, ENABLE);
	
	GPIO_PullResistorConfig(GPIO_PORT_MODE_SW, GPIO_PIN_MODE_SW_2, GPIO_PR_UP);
	GPIO_DirectionConfig(GPIO_PORT_MODE_SW, GPIO_PIN_MODE_SW_2, GPIO_DIR_IN);
	GPIO_InputConfig(GPIO_PORT_MODE_SW, GPIO_PIN_MODE_SW_2, ENABLE);
}

// Foot part GPIO initialize
void Foot_GPIO_Setup()
{
	GPIO_PullResistorConfig(GPIO_PORT_FLEFT, GPIO_PIN_FLEFT, GPIO_PR_UP);
	GPIO_DirectionConfig(GPIO_PORT_FLEFT, GPIO_PIN_FLEFT, GPIO_DIR_IN);
	GPIO_InputConfig(GPIO_PORT_FLEFT, GPIO_PIN_FLEFT, ENABLE);
	
	GPIO_PullResistorConfig(GPIO_PORT_FRIGHT, GPIO_PIN_FRIGHT, GPIO_PR_UP);
	GPIO_DirectionConfig(GPIO_PORT_FRIGHT, GPIO_PIN_FRIGHT, GPIO_DIR_IN);
	GPIO_InputConfig(GPIO_PORT_FRIGHT, GPIO_PIN_FRIGHT, ENABLE);
}

// get total vibrate info
u8 GetVibrateCode()
                              {
	u8 i;
	u8 result = 0;
	
	// get detection info
	for (i = 0; i < 4; i++)
	{
		Vibrate_Status_prv[i] = Vibrate_Status_now[i];
		Vibrate_Status_now[i] = Vibrate_Detect(EID_VIBR[i]);
		if (Vibrate_Status_now[i] == SET && Vibrate_Status_prv[i] == RESET)
		{
			result |= (1 << i);
		}
	}
	return result;
}

// Get Touch Module info
u8 GetTouchKey()
{
	sprintf((char*)cmd, "%c%c", 0x0D,0x00);
	LS_Transmit(0x1d1, 2, cmd);
	ret = LS_Receive(&id, &len, buf);
	if (ret == 0 && buf[0] == 0x00)
		return buf[1];
	else
		return 0xff;
	
}

// get Mode SW info
u8 GetModeSW()
{
	u8 result;
	result = (GPIO_ReadInBit( SWGPIO_Port[1], GPIO_Pin_SW[1]) > 0) ? 1 : 0;
	result |= (GPIO_ReadInBit( SWGPIO_Port[0], GPIO_Pin_SW[0]) > 0) ? 2 : 0;
	return result;
}

// Get Next Game Mode
u8 GetGameMode(u8 SW_Data)
{
	if (SW_Data == 1)
	{
		if (GameMode != 0)
		{
				L_Audio_Play(AUDIO_TYPE_VOICE, SONG_ID_IDLE_MODE_ENG);
		}
		return 0;
	}
	if (SW_Data == 3)
	{
		if (GameMode != 1)
		{
				L_Audio_Play(AUDIO_TYPE_VOICE, SONG_ID_TAP_MODE_ENG);
		}
		return 1;
	}
	if (SW_Data == 2)
	{
		if (GameMode != 2 && GameMode != 3)
		{
			L_Audio_Play(AUDIO_TYPE_VOICE, SONG_ID_SONG_MODE_ENG);
		}
		if (GameMode == 3)
		{
			return 3;
		}
		else
		{
			return 2;
		}
	}
	
	return 4;
}

// get total GPIO info
u8 GetGPIOCode()
{
	u8 i;
	u8 result = 0;
	
	// get GPIO info
	for (i = 0; i < 2; i++)
	{		
		FGPIO_Status_prv[i] = FGPIO_Status_now[i];
		FGPIO_Status_now[i] = GPIO_ReadInBit(FGPIO_Port[i], FGPIO_Pin[i]);
		if (FGPIO_Status_prv[i] == SET && FGPIO_Status_now[i] == RESET)
		{
			result |= (1 << i);
		}
	}
	return result;
}

// Queue Push
void QueuePush(u8 EID, u32 BeatTime)
{
	Queue[EID].EID = EID;
	Queue[EID].Tapped = 0;
	Queue[EID].BeatTime = BeatTime;
}

// Queue Pop
void QueuePop(u8 EID)
{
	Queue[EID].EID = 0;
	Queue[EID].Tapped = 0;
	Queue[EID].BeatTime = SongBeatEnd[0];
}

// Queue Check EID is empty
u8 QueueEIDIsEmpty(u8 EID)
{
	if (Queue[EID].EID == 0)
		return 1;
	return 0;
}
	

// Queue judgement process
void QueueProcess(u8 VibrateCode, u8 GPIOCode)
{
	u8 i;
	// Vibrate part
	for (i = EID_LED_1; i <= EID_LED_4; i++)
	{
		// if led had been triggered, EID won't be 0
		if (Queue[i].EID != 0)
		{ 
			// Tapped
			if (Queue[i].Tapped == 1)
			{
				if ((BeatCounter >= (Queue[i].BeatTime + LED_END_TIMING)) && 
						(BeatCounter <= (Queue[i].BeatTime + LED_END_TIMING + 10)))
				{
					ClearSingleLED(i);
					QueuePop(i);
				}
			}
			// Not Tapped
			else
			{
				// Vibrate detect
				if (VibrateCode & (1 << (i-1)))
				{
					Queue[i].Tapped = 1;
					Queue[i].BeatTime = BeatCounter;
					LED_Multi_Rotate(i, LED_END_HOLD_TIME, LED_ROTATE_SINGLE);
				}
				// clear queue
				else if ((BeatCounter >= (Queue[i].BeatTime + 2*LED_TIMEOUT_TIMING)) && 
								(BeatCounter <= (Queue[i].BeatTime + 2*LED_TIMEOUT_TIMING + 10)))
				{
					QueuePop(i);
					ClearAllLED();
					GameMode = 4;
					AudioCnt = 0;
					return;
				}
				// Timeout detect
				else if ((BeatCounter >= (Queue[i].BeatTime + LED_TIMEOUT_TIMING)) && 
								(BeatCounter <= (Queue[i].BeatTime + LED_TIMEOUT_TIMING + 10)))
				{
					BeatLEDControl(i, LED_DIRECTION_CLOSE);
				}
			}
			
			
		}
	}
	// GPIO part
	for (i = EID_LED_5; i <= EID_LED_6; i++)
	{
		// if led had been triggered, EID won't be 0
		if (Queue[i].EID != 0)
		{
			     
			// Tapped
			if (Queue[i].Tapped == 1)
			{
				if ((BeatCounter >= (Queue[i].BeatTime + LED_END_TIMING)) && 
						(BeatCounter <= (Queue[i].BeatTime + LED_END_TIMING + 10)))
				{
					ClearSingleLED(i);
					QueuePop(i);
				}
			}
			// Not Tapped
			else
			{
				// GPIO detect
				if (GPIOCode & (1 << (i-5)))
				{
					Queue[i].Tapped = 1;
					Queue[i].BeatTime = BeatCounter;
					LED_Multi_Rotate(i, LED_END_HOLD_TIME, LED_ROTATE_SINGLE);
				}  
				// clear queue
				else if ((BeatCounter >= (Queue[i].BeatTime + 2*LED_TIMEOUT_TIMING)) && 
								(BeatCounter <= (Queue[i].BeatTime + 2*LED_TIMEOUT_TIMING + 10)))
				{
					QueuePop(i);
					ClearAllLED();
					GameMode = 4;
					AudioCnt = 0;
					return;
				}
				// Timeout detect
				else if ((BeatCounter >= (Queue[i].BeatTime + LED_TIMEOUT_TIMING)) && 
								(BeatCounter <= (Queue[i].BeatTime + LED_TIMEOUT_TIMING + 10)))
				{
					BeatLEDControl(i, LED_DIRECTION_CLOSE);
				}
			}
			
		}
		
	}
}

// Queue initialize
void QueueInit()
{
	u8 i;
	for (i = 0; i < QUEUE_MAX; i++)
	{
		Queue[i].EID = 0;
		Queue[i].Tapped = 0;
		Queue[i].BeatTime = 0;
	}
}


void SetBLEName()
{
	sprintf((char*)cmd, "%s", "Tap it!");
	BLE_Set_Name((char*)cmd);
}

void BLESetWhiteList()
{
	u8 TX_data[19];
	
	TX_data[0] = 0x25;
	TX_data[1] = 0x37;
  TX_data[2] = 0x06;
	
  TX_data[3] = 0x00;
  TX_data[4] = 0x00;
  TX_data[5] = 0x00;
	TX_data[6] = 0x00;	
  TX_data[7] = 0x00;
  TX_data[8] = 0x00;
  
  BC76xxSPI_write_fifo(TX_data, TX_data[2]+3);
}

void BLESetupInterval()
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
void BLE_CustomInit()
{
	SetBLEName();
	BLESetWhiteList();
	BLESetupInterval();
}


u8 BLE_IsConnected()
{
	u16 ret;
	ret = BC76xxSPI_read_register(0x05);
	return ((ret & 0x02) == 0x02);
}


// Tap mode detect
void TapModeProcess(u8 VibrateCode, u8 GPIOCode)
{
	uint16_t i;
	uint8_t LED_Cnt[6] = {0, 0, 0, 0, 0, 0};
	
	// vibrate
	LED_Cnt[0] = ((VibrateCode & 0x01) == 0x01);
	LED_Cnt[1] = ((VibrateCode & 0x02) == 0x02);
	LED_Cnt[2] = ((VibrateCode & 0x04) == 0x04);
	LED_Cnt[3] = ((VibrateCode & 0x08) == 0x08);
	
	
	
	// GPIO
	LED_Cnt[4] = ((GPIOCode & 0x01) == 0x01);
	LED_Cnt[5] = ((GPIOCode & 0x02) == 0x02);
	
	// LED display
	for (i = 0; i < 6; i++){
		if (LED_Cnt[i])
		{
			QueuePush(EID_LED[i], BeatCounter);
		}
	}
	QueueProcess(VibrateCode, GPIOCode);
}

int main(void)
{
	////////////////////////////////////////////////////
	//System Initialization Area
	////////////////////////////////////////////////////  
	RETARGET_Configuration();		//USB Initialization
	GPIO_PortInit();						//GPIO pin & related basic function Initialization
	URBus_Init();								//UART Bus Initialization
	Timer_Init();  							//System timer Initialization
	BC76XX_Init();							//BLE Initialization
	IAP_Init();									//IAP flag & buffer Initialization
	MIDI_FuncInit();						//MIDI Initialization
	
	////////////////////////////////////////////////////
	//User Initialization Area
	////////////////////////////////////////////////////
	int i;
	u8 DelayCnt = 0;
	u8 VibrateCode = 0x00;
	u8 GPIOCode = 0x00;
	u8 EID_Rand = 0;
	u8 BT_Received = 0;
	u8 BT_buffer[15];
	u8 Song_Mode_Id;
	u8 Song_Mode_Tempo;
	u8 Song_Mode_Diff;
	u8 SongBeatMapTap;
	
	// system startup reset all device
	System_Reset();
	
	// Bluetooth custom init
	BLE_CustomInit();
	BLE_Power_Down();
	// Mode Switch Setup
	ModeGPIOSetup();
	
	// Foot part GPIO initialize
	Foot_GPIO_Setup();
	
	// Audio Volume setup
	L_Audio_Vol(Volume);
	
  //vibrate detection sensitivity set to 1
	for (i = 0; i < 4; i++)
	{
		Vibrate_Sensitivity_Set(EID_VIBR[i],  1);
	}
	
	//RGB LED init show
	for (i = 0; i < 6; i++)
	{
		LED_Multi_Rotate(EID_LED[i], LED_DEFAULT_HOLD_TIME, LED_ROTATE_MULTI);
	}
	
	while (1)
	{
		////////////////////////////////////////////////////
		//System Coding Area
		////////////////////////////////////////////////////    
		if(HT_CKCU->APBCCR1 & (1 << 4))
			WDT_Restart();
		IAP_Handler();
		BLE_process();
		MIDI_Procrss();
		
		////////////////////////////////////////////////////
		//User Coding Area
		////////////////////////////////////////////////////
		// Idle Mode
		if (GameMode == 0)
		{
			// detect Mode
			GameMode = GetGameMode(GetModeSW());
			if (GameMode == 1)
			{
				ClearAllLED();
			}
			else if (GameMode != 0)
			{
				for (i = 0; i < 6; i++)
				{
					LED_Multi_Rotate(EID_LED[i], LED_DEFAULT_HOLD_TIME, LED_ROTATE_MULTI);
					LED_Multi_Rotate(EID_LED[i], LED_DEFAULT_HOLD_TIME, LED_ROTATE_MULTI);
				}
			}
			AudioCnt = 0;
			
		}
		// Game Mode 1
		else if (GameMode == 1)
		{
			AudioCnt = 0;
			
			// vibration detect
			VibrateCode = GetVibrateCode();
			// foot GPIO detect
			GPIOCode = GetGPIOCode();
			
			// Tap Mode detect and process
			TapModeProcess(VibrateCode, GPIOCode);
			
			// LED display
			//LedTest(VibrateCode, GPIOCode);	
			
			GameMode = GetGameMode(GetModeSW());
			if (GameMode != 1)
			{
				for (i = 0; i < 6; i++)
				{
					LED_Multi_Rotate(EID_LED[i], LED_DEFAULT_HOLD_TIME, LED_ROTATE_MULTI);
				}
				for (i = 0; i < 6; i++)
				{
					LED_Multi_Rotate(EID_LED[i], LED_DEFAULT_HOLD_TIME, LED_ROTATE_MULTI);
				}
			}
			
		}
		// connect bluetooth
		else if (GameMode == 2)
		{	
			GameMode = GetGameMode(GetModeSW());
			if (GameMode == 1)
			{
				ClearAllLED();
			}
			else if (GameMode != 2)
			{
				BLE_Power_Down();
				for (i = 0; i < 6; i++)
				{
					LED_Multi_Rotate(EID_LED[i], LED_DEFAULT_HOLD_TIME, LED_ROTATE_MULTI);
					LED_Multi_Rotate(EID_LED[i], LED_DEFAULT_HOLD_TIME, LED_ROTATE_MULTI);
				}
			}
			else
			{
				if (AudioCnt == 0)
				{
					if (L_Audio_Finish() == ENABLE)
					{
						AudioCnt++;
						L_Audio_Play(AUDIO_TYPE_VOICE, SONG_ID_CONNECTING_BT_ENG);
						BLE_Power_Up();
					}
					
				}
				else if (AudioCnt == 1)
				{
					if (L_Audio_Finish() == ENABLE)
					{
						BT_Connected = 0;
						SongMode_Setup = 0;
						BT_Received = 0;
						AudioCnt++;
						
					}
				}
				else if (AudioCnt == 2 && SongMode_Setup == 1)
				{
					// Initialize
					QueueInit();
					ClearAllLED();
					ClearAllLED();
					pSongBeatMapTiming = (u32*)SongBeatMapTimingArray[Song_Mode_Id];
					
					
					SongMode_Setup = 0;
					GameMode = 3;
					AudioCnt = 0;
					// set up random seed
					srand(RandCnt);
					// Beat counter reset
					BeatCounter = 0;
					L_Audio_Play(Song_Type[Song_Mode_Id], Song_Id[Song_Mode_Id]);
					// Map Counter reset
					MapCnt = 0;
					// next game mode
				}
				else{
					if (BT_Connected == 0 && BLE_IsConnected() == 1)
					{
						L_Audio_Play(AUDIO_TYPE_VOICE, SONG_ID_BT_CONNECTED_ENG);
						BT_Connected = 1;
					}
					else if (BT_Connected == 1)
					{
						if (BLE_IsConnected() == 0)
						{
							AudioCnt = 0;
							BT_Connected = 0;
						}
						else
						{
							if (Get_BLE_RX_Length() > 0)
							{
								
								BLE_Read_Buffer(buf, 1);
								//IAP_UART_Send(buf, 1);
								if (BT_Received != 0)
								{
									BT_buffer[BT_Received] = buf[0];
									BT_Received++;
								}
								else if (BT_Received == 0 && buf[0] == 'S')
								{
									BT_buffer[BT_Received] = buf[0];
									BT_Received++;
								}
								if (BT_Received == 9)
								{
									//IAP_UART_Send(BT_buffer, 9);
									SongMode_Setup = 2;
									Song_Mode_Id = BT_buffer[2] - 1;
									Song_Mode_Tempo = BT_buffer[5];
									Song_Mode_Diff = BT_buffer[8];
									L_Audio_Play(AUDIO_TYPE_VOICE, SONG_ID_GAME_START_ENG);
									BT_Received = 0;
								}
								
							}
							if (L_Audio_Finish() == ENABLE && SongMode_Setup == 2)
							{
								SongMode_Setup = 1;
								BLE_Power_Down();
							}
						}
					}
						
					
				}
				
			}
		
		}
		// Game mode 2
		else if (GameMode == 3)
		{
			// start game mode
			if (BeatCounter >= SongBeatEnd[Song_Mode_Id])
			{
				GameMode = 4;
				AudioCnt = 0;
			}
			// judgement Map timing
			else if (BeatCounter >= pSongBeatMapTiming[MapCnt] && BeatCounter < pSongBeatMapTiming[MapCnt+1])
			{
				if (MapCnt % Song_Mode_Tempo == 0)
				{
					i = 0;
					SongBeatMapTap = (rand() % Song_Mode_Diff) + 1;
					// mulit note push into queue
					while (i < SongBeatMapTap)
					{
						// 3 note at one time
						if (SongBeatMapTap> 2)
						{
							// 2 hand
							if (i < 2)
							{
								//do{
									EID_Rand = (rand() % 4) + 1;
								//}while (QueueEIDIsEmpty(EID_Rand) == 0);
							}
							// 1 foot
							else
							{
								//do{
									EID_Rand = (rand() % 2) + 5;
								//}while (QueueEIDIsEmpty(EID_Rand) == 0);
							}
							BeatLEDControl(EID_Rand, LED_DIRECTION_OPEN);
							QueuePush(EID_Rand, BeatCounter);
							i++;
						}
						else
						{
							//do{
								EID_Rand = (rand() % 6) + 1;
							//}while (QueueEIDIsEmpty(EID_Rand) == 0 && i < 3);
							BeatLEDControl(EID_Rand, LED_DIRECTION_OPEN);
							QueuePush(EID_Rand, BeatCounter);
							i++;
						}
						
					}
				}
				// Next Map Note
				MapCnt++;
			}
			// vibration detect
			VibrateCode = GetVibrateCode();
			// foot GPIO detect
			GPIOCode = GetGPIOCode();	
			// Queue Judgement process
			QueueProcess(VibrateCode, GPIOCode);
			
			GameMode = GetGameMode(GetModeSW());
			if (GameMode == 1)
			{
				ClearAllLED();
			}
			else if (GameMode != 3)
			{
				for (i = 0; i < 6; i++)
				{
					LED_Multi_Rotate(EID_LED[i], LED_DEFAULT_HOLD_TIME, LED_ROTATE_MULTI);
					LED_Multi_Rotate(EID_LED[i], LED_DEFAULT_HOLD_TIME, LED_ROTATE_MULTI);
				}
			}
			
		}
		// default process
		else
		{
			AudioCnt = 0;
			// detect Mode
			GameMode = GetGameMode(GetModeSW());
			//RGB LED init show
			for (i = 0; i < 6; i++)
			{
				LED_Multi_Rotate(EID_LED[i], LED_DEFAULT_HOLD_TIME, LED_ROTATE_MULTI);
			}
		}
		if (DelayCnt >= 5)
		{
			DelayCnt = 0;
			Temp = GetTouchKey();
			if (Temp != 0xff)
			{				
				Volume = (Temp << 1) | 1;
				L_Audio_Vol(Volume);
			}
		}
		else
		{
			DelayCnt++;
		}
	}
}
