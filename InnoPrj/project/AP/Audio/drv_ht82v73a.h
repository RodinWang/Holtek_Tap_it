/* Define to prevent recursive inclusion -------------------------------------------------------------------*/
#ifndef __DRV_VOICE_H
#define __DRV_VOICE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"

/* Settings ------------------------------------------------------------------------------------------------*/
#define VOICE_BUF_SIZE       			 (320)

#define	VoiceDataAddress					(0x8000)
#define	PWMRatioValue							3				
#define	UseIAPUpdate							0

#define	CFG_SAMPLE_TM							BFTM0
#define CFG_VOIVE_TM              PWM0   // GPTMx or PWMx or MCTMx
#define CFG_VOICE_PWM1_PORT       B
#define CFG_VOICE_PWM1_PIN        15
#define CFG_VOICE_PWM1_TM_CH      2
#define CFG_VOICE_PWM2_PORT       C
#define CFG_VOICE_PWM2_PIN        0
#define CFG_VOICE_PWM2_TM_CH      3

#define SAMPLE_TM                 STRCAT2(HT_,            CFG_SAMPLE_TM)
#define SAMPLE_TM_IRQn            STRCAT2(CFG_SAMPLE_TM,   _IRQn)
#define SAMPLE_TM_IRQHandler      STRCAT2(CFG_SAMPLE_TM,   _IRQHandler)
#define VOICE_TM                  STRCAT2(HT_,            CFG_VOIVE_TM)
#define VOICE_TM_IRQn             STRCAT2(CFG_VOIVE_TM,   _IRQn)
#define VOICE_TM_IRQHandler       STRCAT2(CFG_VOIVE_TM,   _IRQHandler)
#define VOICE_PWM1_PORT_ID        STRCAT2(GPIO_P,         CFG_VOICE_PWM1_PORT)
#define VOICE_PWM1_PIN            STRCAT2(GPIO_PIN_,      CFG_VOICE_PWM1_PIN)
#define VOICE_PWM2_PORT_ID        STRCAT2(GPIO_P,         CFG_VOICE_PWM2_PORT)
#define VOICE_PWM2_PIN            STRCAT2(GPIO_PIN_,      CFG_VOICE_PWM2_PIN)
#define VOICE_PWM1                STRCAT3(CH,             CFG_VOICE_PWM1_TM_CH,     CCR)
#define VOICE_PWM2                STRCAT3(CH,             CFG_VOICE_PWM2_TM_CH,     CCR)
#define VOICE_PWM1_TM_CH          STRCAT2(TM_CH_,         CFG_VOICE_PWM1_TM_CH)
#define VOICE_PWM2_TM_CH          STRCAT2(TM_CH_,         CFG_VOICE_PWM2_TM_CH)

/* Exproted constants --------------------------------------------------------------------------------------*/
#define VOICE_STATE_0           (0x00)
#define VOICE_STATE_1_IDLE      (0x01)
#define VOICE_STATE_2_PAUSE     (0x02)
#define VOICE_STATE_3_PLAY      (0x03)
#define VOICE_STATE_4_PLAY_LAST (0x04)
#define VOICE_STATE_5_PLAY_END  (0x05)

/* Exported typedefs ---------------------------------------------------------------------------------------*/
typedef struct
{
  vu8 Current;
  vu8 Backup;  
} VOICE_STATE_TypeDef;

/* Exported variables --------------------------------------------------------------------------------------*/ 

/* Exported functions --------------------------------------------------------------------------------------*/
void Audio_Init(void);
void Audio_Process(void);
u8 Audio_Play(u16 num, u8 volume);
u8 Audio_Pause(void);
u8 Audio_Resume(void);
u8 Audio_Stop(void);
u8 Audio_Volume(u8 volume);
bool Audio_Finish(void);

#ifdef __cplusplus
}
#endif

#endif
