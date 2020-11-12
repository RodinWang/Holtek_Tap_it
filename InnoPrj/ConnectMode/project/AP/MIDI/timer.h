

#ifndef		_TIMER_H
#define		_TIMER_H

#include	"ht32.h"




void BFTM_Config(void);
void BFTM1_TEMPO_Config(u8);   
void User_TimerInterrupt_2ms(void);

void User_TimerIRQ_Beat_48(void);


extern u16		R_Time_Delay_Demo_Replay;


#endif
