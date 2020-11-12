/*********************************************************************************************************//**
 * @file    drv_ht82v742_sbc.c
 * @brief   This file contains all body.
 * @version
 *
 ************************************************************************************************************/

/* Includes ------------------------------------------------------------------------------------------------*/
#include "drv_ht82v742.h"
#include "defs.h"
#include "typedef.h"
#include "libG722.h"
#include "qspi_flash_MX25L12835F.h"

#define	VOLUME_DB_P11         14533 
#define	VOLUME_DB_P10p5       13720 
#define	VOLUME_DB_P10         12953 
#define	VOLUME_DB_P9p5        12228 
#define	VOLUME_DB_P9          11544 
#define	VOLUME_DB_P8p5        10898 
#define	VOLUME_DB_P8          10289 
#define	VOLUME_DB_P7p5        9713 
#define	VOLUME_DB_P7          9170 
#define	VOLUME_DB_P6p5        8657 
#define VOLUME_DB_P6          8173 
#define VOLUME_DB_P5p5        7715 
#define VOLUME_DB_P5          7284 
#define VOLUME_DB_P4p5        6876 
#define VOLUME_DB_P4          6492 
#define VOLUME_DB_P3p5        6129 
#define VOLUME_DB_P3          5786 
#define VOLUME_DB_P2p5        5462 
#define VOLUME_DB_P2          5157 
#define VOLUME_DB_P1p5        4868 
#define VOLUME_DB_P1          4596 
#define VOLUME_DB_P0p5        4339 
#define VOLUME_DB_ZERO        4096 
#define VOLUME_DB_M0p5        3867 
#define VOLUME_DB_M1          3651 
#define VOLUME_DB_M1p5        3446 
#define VOLUME_DB_M2          3254 
#define VOLUME_DB_M2p5        3072 
#define VOLUME_DB_M3          2900 
#define VOLUME_DB_M3p5        2738 
#define VOLUME_DB_M4          2584 
#define VOLUME_DB_M4p5        2440 
#define VOLUME_DB_M5          2303 
#define VOLUME_DB_M5p5        2175 
#define VOLUME_DB_M6          2053 
#define VOLUME_DB_M6p5        1938 
#define VOLUME_DB_M7          1830 
#define VOLUME_DB_M7p5        1727 
#define VOLUME_DB_M8          1631 
#define VOLUME_DB_M8p5        1539 
#define VOLUME_DB_M9          1453 
#define VOLUME_DB_M9p5        1372 
#define VOLUME_DB_M10         1295 
#define VOLUME_DB_M10p5       1223 
#define VOLUME_DB_M11         1154 
#define VOLUME_DB_M11p5       1090 
#define VOLUME_DB_M12         1029 
#define VOLUME_DB_M12p5       971 
#define VOLUME_DB_M13         917 
#define VOLUME_DB_M13p5       866 
#define VOLUME_DB_M14         817 
#define VOLUME_DB_M14p5       772 
#define VOLUME_DB_M15         728 
#define VOLUME_DB_M15p5       688 
#define VOLUME_DB_M16         649 
#define VOLUME_DB_M16p5       613 
#define VOLUME_DB_M17         579 
#define VOLUME_DB_M17p5       546 
#define VOLUME_DB_M18         516 
#define VOLUME_DB_M18p5       487 
#define VOLUME_DB_M19         460 
#define VOLUME_DB_M19p5       434 
#define VOLUME_DB_M20         410 
#define VOLUME_DB_M20p5       387 
#define VOLUME_DB_M21         365 
#define VOLUME_DB_M21p5       345 
#define VOLUME_DB_M22         325 
#define VOLUME_DB_M22p5       307 
#define VOLUME_DB_M23         290 
#define VOLUME_DB_M23p5       274 
#define VOLUME_DB_M24         258 
#define VOLUME_DB_M24p5       244 
#define VOLUME_DB_M25         230 
#define VOLUME_DB_M25p5       217 
#define VOLUME_DB_M26         205 
#define VOLUME_DB_M26p5       194 
#define VOLUME_DB_M27         183 
#define VOLUME_DB_M27p5       173 
#define VOLUME_DB_M28         163 
#define VOLUME_DB_M28p5       154 
#define VOLUME_DB_M29         145 
#define VOLUME_DB_M29p5       137 
#define VOLUME_DB_M30         130 
#define VOLUME_DB_M30p5       122 
#define VOLUME_DB_M31         115 
#define VOLUME_DB_M31p5       109 
#define VOLUME_DB_M32         103 

#define VOLUME_DB_RESOLUTION  87

/* Private function prototypes -----------------------------------------------------------------------------*/
static void SoftwareAmp(s16* pBuf);

/* Private variables ---------------------------------------------------------------------------------------*/
#define VOLUME_DB_TABLE_SIZE  12
#define VOLUME_DB_ZERO_INDEX  6
static s16 const VolumeTable[VOLUME_DB_TABLE_SIZE] = 
{  
  0,  
  VOLUME_DB_M25,    
  VOLUME_DB_M20,   
  VOLUME_DB_M15,   
  VOLUME_DB_M10,   
  VOLUME_DB_M5,     
  VOLUME_DB_ZERO,   
  VOLUME_DB_P2,   
  VOLUME_DB_P4,   
  VOLUME_DB_P6,
  VOLUME_DB_P8,
  VOLUME_DB_P10,
};

static s16 volume_db = VOLUME_DB_ZERO;
static vu8 CurrentPingPongIndex;
static u8 PreviousPingPongIndex;
static vu16 BufferIndex;
static u16 PingPongBuffer[2][VOICE_BUF_SIZE];
static u32 PlayPtr;
static u32 PlayLength;
static u16 VoicePerloadValue;

static const G722_CODEC_CTL cctl =
{
  16000,              //bit_rate;
  7000,               //bandwidth;
  16000/50,           //number_of_bits_per_frame;
  NUMBER_OF_REGIONS,  //number_of_regions;
  16000/50            //frame_size;  
};

static G722_DEC_CTX ctx;

/* Global variables ----------------------------------------------------------------------------------------*/
volatile VOICE_STATE_TypeDef gVOICE_State;

/*********************************************************************************************************//**
 * @brief   This function handles BFTM0 interrupt.
 * @retval  None
 ************************************************************************************************************/
void BFTM0_IRQHandler(void)
{
	BFTM_ClearFlag(SAMPLE_TM);
  if(gVOICE_State.Current < VOICE_STATE_3_PLAY)
  {
    /*--------------------------------------------------------------------------------------------------------*/
    /* DSB instruction is added in this function to ensure the write operation which is for clearing interrupt*/
    /* flag is actually completed before exiting ISR. It prevents the NVIC from detecting the interrupt again */
    /* since the write register operation may be pended in the internal write buffer of Cortex-Mx when program*/
    /* has exited interrupt routine. This DSB instruction may be masked if this function is called in the     */
    /* beginning of ISR and there are still some instructions before exiting ISR.                             */
    /*--------------------------------------------------------------------------------------------------------*/
    __DSB();  
    return;
  }
  
  if(gVOICE_State.Current == VOICE_STATE_5_PLAY_END)
  {
    VOICE_TM->VOICE_PWM1 = 0;
    VOICE_TM->VOICE_PWM2 = 0;
		BFTM_IntConfig(SAMPLE_TM, DISABLE);
    gVOICE_State.Current = VOICE_STATE_1_IDLE;
  }
  else
  {
    u16 dwData = PingPongBuffer[CurrentPingPongIndex][BufferIndex];
		u16 wDAC_Data = dwData;

		dwData = ((dwData&0x7FFF)*VoicePerloadValue)>>15;
		if(wDAC_Data & 0x8000)
		{
			VOICE_TM->VOICE_PWM1 = dwData;
			VOICE_TM->VOICE_PWM2 = 0;
		}else
		{
			VOICE_TM->VOICE_PWM1 = 0;
			VOICE_TM->VOICE_PWM2 = VoicePerloadValue-dwData;
		}	
    TM_SetCounter(VOICE_TM, 0);
		
    BufferIndex++;
    if(BufferIndex >= VOICE_BUF_SIZE)
    {
      if(gVOICE_State.Current == VOICE_STATE_4_PLAY_LAST)
      {
        gVOICE_State.Current = VOICE_STATE_5_PLAY_END;
      }
      else
      {
        BufferIndex = 0;
        CurrentPingPongIndex = 1 - CurrentPingPongIndex;
      }
    }
  }
}

/*********************************************************************************************************//**
 * @brief Voice initation
 * @param None
 * @retval None
 * @note Sample Timer & Voice Timer 
 ************************************************************************************************************/
void Audio_Init(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}}; 
	TM_OutputInitTypeDef 		GPTM_OutputInitStructure;	
  
	CKCUClock.Bit.AFIO            = 1;
  CKCUClock.Bit.CFG_SAMPLE_TM		= 1;	
  CKCUClock.Bit.CFG_VOIVE_TM 		= 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
  
  gVOICE_State.Current = VOICE_STATE_1_IDLE;  
	
  BFTM_EnaCmd(SAMPLE_TM, ENABLE);
  NVIC_EnableIRQ(BFTM0_IRQn);

	AFIO_GPxConfig(VOICE_PWM1_PORT_ID, VOICE_PWM1_PIN, AFIO_MODE_13);
	AFIO_GPxConfig(VOICE_PWM2_PORT_ID, VOICE_PWM2_PIN, AFIO_MODE_13);
	
	GPTM_OutputInitStructure.Channel    = VOICE_PWM1_TM_CH;
	GPTM_OutputInitStructure.OutputMode = TM_OM_PWM1;
	GPTM_OutputInitStructure.Control		= TM_CHCTL_ENABLE;
	GPTM_OutputInitStructure.Compare		= 0;
	GPTM_OutputInitStructure.Polarity		= TM_CHP_NONINVERTED;
	GPTM_OutputInitStructure.AsymmetricCompare = 0;
	TM_OutputInit(VOICE_TM, &GPTM_OutputInitStructure);
	
	GPTM_OutputInitStructure.Channel = VOICE_PWM2_TM_CH;
	TM_OutputInit(VOICE_TM, &GPTM_OutputInitStructure);
	
	TM_CHCCRPreloadConfig(VOICE_TM, VOICE_PWM1_TM_CH, ENABLE);	
	TM_CHCCRPreloadConfig(VOICE_TM, VOICE_PWM2_TM_CH, ENABLE);

	TM_Cmd(VOICE_TM, ENABLE);
  
  QSPI_FLASH_Init();
  QSPI_FLASH_RDID();
  QSPI_FLASH_WRR(0x42, 0x7);
}

/*********************************************************************************************************//**
 * @brief Display specific number voice
 * @param num: Number
 * @retval None
 * @note num range = 1 -- all;
 ************************************************************************************************************/
u8 Audio_Play(u16 num, u8 volume)
{
/* internal flash
  u32 pos = VoiceDataAddress;
  u16 type;
  
  if(num > *(u16*)pos || num == 0)
    return;
  gVOICE_State.Current = VOICE_STATE_1_IDLE;
  
  pos += 2+(num-1)*(2+4+4);//all number(2bytes)+type(2bytes)+start_addr(4bytes)+number_size(4bytes)
  type = *(u16*)pos;
  if(type & 0x3)return;
  if((type >> 2) != 3200)return;
  pos += 2; 
  PlayPtr = (s16*)(*(u32*)pos);
  pos += 4;
  PlayLength = *(u32*)pos;
  PlayLength /= 2;//div 2 because PlayLength use s16 unit, and data length use u8 unit
 
  if(PlayLength < cctl.number_of_bits_per_frame >> 4)
    return;
	
  g722decReset(&cctl, &ctx);
  gVOICE_State.Current = VOICE_STATE_3_PLAY;
  PreviousPingPongIndex = CurrentPingPongIndex = 1;
  VOICE_Process();
  CurrentPingPongIndex = 0;
  
  VOICE_Process();
      
  BufferIndex = 0;
	BFTM_IntConfig(SAMPLE_TM, ENABLE);	 
*/
  u8  temp_buffer[32];
  u16 samp_rate;
  u32 pos;
  TM_TimeBaseInitTypeDef 	GPTM_TimeBaseInitStructure;
  
  if(Audio_Volume(volume))
    return 3;
  
  //Find audio data adress and length 
  QSPI_FLASH_BYTE_QOR(temp_buffer, 0, 2);
  if(num > *(u16*)temp_buffer || num == 0)
  {
    return 1;
  }
  gVOICE_State.Current = VOICE_STATE_1_IDLE;
  
  pos = 2+(num-1)*(2+4+4);//all number(2bytes)+type(2bytes)+start_addr(4bytes)+number_size(4bytes)
  QSPI_FLASH_BYTE_QOR(temp_buffer, pos, 10);
  if(*(u16*)temp_buffer & 0x3)return 1;//not surport double channels audio
  samp_rate = *(u16*)temp_buffer >> 2;
  samp_rate *= 5;//the real sample rate
  PlayPtr = temp_buffer[2] | temp_buffer[3] << 8 | temp_buffer[4] << 16 | temp_buffer[5] << 24;
  PlayLength = temp_buffer[6] | temp_buffer[7] << 8 | temp_buffer[8] << 16 | temp_buffer[9] << 24;
  PlayLength /= 2;//div 2 because PlayLength use s16 unit, and data length use u8 unit
 
  if(PlayLength < cctl.number_of_bits_per_frame >> 4)
  {
    return 2;
  }
  
  //Initialize timer depend on sample rate
  BFTM_SetCompare(SAMPLE_TM, (u16)(SystemCoreClock/samp_rate+0.5) - 1);
  VoicePerloadValue = (u16)(SystemCoreClock/(samp_rate*PWMRatioValue)+0.5);
	GPTM_TimeBaseInitStructure.CounterMode   = TM_CNT_MODE_UP;
	GPTM_TimeBaseInitStructure.CounterReload = VoicePerloadValue-1; 
	GPTM_TimeBaseInitStructure.Prescaler		 = 0;
	GPTM_TimeBaseInitStructure.PSCReloadTime = TM_PSC_RLD_UPDATE;
	TM_TimeBaseInit(VOICE_TM, &GPTM_TimeBaseInitStructure);
  
	//Ready to display
  BFTM_IntConfig(SAMPLE_TM, DISABLE);	
  VOICE_TM->VOICE_PWM1 = 0;
  VOICE_TM->VOICE_PWM2 = 0;//if there is no before three instructions, the changing moment will exsit 'bo' noisy
  g722decReset(&cctl, &ctx);
  gVOICE_State.Current = VOICE_STATE_3_PLAY;
  PreviousPingPongIndex = CurrentPingPongIndex = 1;
  Audio_Process();
  CurrentPingPongIndex = 0;
  Audio_Process();
   
  BufferIndex = 0;
  BFTM_SetCounter(SAMPLE_TM, 0);
	BFTM_IntConfig(SAMPLE_TM, ENABLE);	
  return 0;
}

/*********************************************************************************************************//**
 * @brief Deal with the voice process
 * @param None
 * @retval None
 * @note Shoule put it in main while loop
 ************************************************************************************************************/
static s16 Audio_buffer[20];
void Audio_Process(void)
{
  if(gVOICE_State.Current == VOICE_STATE_3_PLAY)
  {
    if(CurrentPingPongIndex == PreviousPingPongIndex)
    {
      s16 number_of_16bit_words_per_frame = cctl.number_of_bits_per_frame >> 4;
  
      if(PlayLength < number_of_16bit_words_per_frame)
      {
        gVOICE_State.Current = VOICE_STATE_4_PLAY_LAST;        
      }
      else
      {
        PreviousPingPongIndex = 1 - PreviousPingPongIndex;
        QSPI_FLASH_BYTE_QOR((u8*)Audio_buffer, (u32)PlayPtr, 40);
        g722dec(&cctl, &ctx, Audio_buffer, (s16*)PingPongBuffer[PreviousPingPongIndex]);  

        PlayPtr += number_of_16bit_words_per_frame*2;
        PlayLength -= number_of_16bit_words_per_frame;

        SoftwareAmp((s16*)&PingPongBuffer[PreviousPingPongIndex][0]);
      }
    }
  }
}

/*********************************************************************************************************//**
 * @brief Pause voice display
 * @param None
 * @retval None
 ************************************************************************************************************/
u8 Audio_Pause(void)
{
  if(gVOICE_State.Current >= VOICE_STATE_3_PLAY)
  {
		BFTM_IntConfig(SAMPLE_TM, DISABLE);
    VOICE_TM->VOICE_PWM1 = 0;
    VOICE_TM->VOICE_PWM2 = 0;
    gVOICE_State.Backup = gVOICE_State.Current;
    gVOICE_State.Current = VOICE_STATE_2_PAUSE;
    return 0;
  }
  return 1;
}

/*********************************************************************************************************//**
 * @brief Contiune voice display
 * @param None
 * @retval None
 ************************************************************************************************************/
u8 Audio_Resume(void)
{
  if(gVOICE_State.Current == VOICE_STATE_2_PAUSE)
  {
    gVOICE_State.Current = gVOICE_State.Backup;

		BFTM_IntConfig(SAMPLE_TM, ENABLE);
    return 0;
  }
  return 1;
}

/*********************************************************************************************************//**
 * @brief stop voice display 
 * @param None
 * @retval None
 ************************************************************************************************************/
u8 Audio_Stop(void)
{
  gVOICE_State.Current = VOICE_STATE_5_PLAY_END;
  return 0;
}

/*********************************************************************************************************//**
 * @brief Check the current voice display finish
 * @param None
 * @retval TRUE or FALSE
 ************************************************************************************************************/
bool Audio_Finish(void)
{
  if(gVOICE_State.Current == VOICE_STATE_1_IDLE)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/*********************************************************************************************************//**
 * @brief Setting volume
 * @param volume:volume level
 * @retval None
 * @note number of VOLUME_DB_TABLE_SIZE level;VOLUME_DB_ZERO_INDEX mean source voice
 ************************************************************************************************************/
u8 Audio_Volume(u8 volume)
{
  if(volume > VOLUME_DB_TABLE_SIZE)
  {
    return 1;
  }
  if(volume == VOLUME_DB_TABLE_SIZE)volume--;
  
  volume_db = VolumeTable[volume];
  return 0;
}

/* Private functions ---------------------------------------------------------------------------------------*/

static void SoftwareAmp(s16* pBuf)
{
  int i;
  
  for(i=0 ; i<VOICE_BUF_SIZE ; i++)
  {
    s32 data = (s32)pBuf[i];
    
    data = (data * volume_db) / VOLUME_DB_ZERO;
    if(data > 32767)
      data = 32767;

    if(data < -32767)
      data = -32768;
      
    data ^= 0x8000;    
    pBuf[i] = data;
  }
}
