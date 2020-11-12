
#ifndef _BLEPROCESS_H_
#define _BLEPROCESS_H_

#include "ht32.h"
#include	"bc76xx.h"

enum
{
   _BLE_START_ = 0x00,
    _BLE_ADVERTISE_,
   _BLE_POWER_UP_ = 0x80,
   _BLE_PWRUP_DELAY_L_,
   _BLE_PWRUP_DELAY_H_,	
	_BLE_PWRUP_EXAMINE_,
	_BLE_EXAMINE_RESPOND_,
	_BLE_IO_ACTIVE_INIT_ = 0x90,
	_BLE_IO_ACTIVE_RESPOND_,	
   _BLE_UUID_INIT_ = 0xA0,
   _BLE_UUID_RESPOND_,
	_BLE_PARAM_MODIFY_1_ = 0xB0,
	_BLE_PARAM_MODIFY_2_,
	_BLE_PARAM_MODIFY_3_,
	_BLE_PARAM_MODIFY_4_,
	_BLE_PARAM_MODIFY_5_,	
	_BLE_PARAM_MODIFY_6_,
	_BLE_PWR_ON_FINISH_ = 0xDE,
   _BLE_PWR_ON_SUCCESS_ = 0xE0,
	_BLE_POWER_DOWN_ = 0xFD,
   _BLE_PWR_ON_ERROR_ = 0xFF
};
enum
{
  _ADVER_IDLE_ = 0x00,
  _ADVER_START_,
  _ADVER_DELAY_,
  _ADVER_WORKING_
};

#define	PWRUP_DELAY_TIME		500			/* BC76xx power on delay time(ms) */
#define  EVENT_RETURN_TIME    5           /* 5ms */
#define	EPWR_DELAY_TIME		50				/* EEPROM write delay time(ms) */
#define ADVERTISE_SIZE    31

typedef __packed struct
{
  bool adverCount_flag;
  bool updateAdver_flag;
  u8 adver_stage;
  u8 adver_len;
  u8 adver_buffer[ADVERTISE_SIZE];
  u32 adver_cnt;
}BLE_Advertise_Struct;

typedef __packed struct 
{	
	u8		state;
	bool	event_finish;
	bool	connect;	
	u16	timer_counter;
	PARSER_OPERATE rcvop;
  BLE_Advertise_Struct adver_event;
}BLE_OPERATE;


extern BLE_OPERATE BLEoperate;

void 	BLE_param_configure(void);
void 	BLE_enter_power_up(void);
void 	BLE_enter_power_save(void);
void 	BLE_timer_process(void);
void 	BLE_interval_packet(u16 min_intv,u16 max_intv,CTRL_PACKAGE *buf);
void 	BLE_advertising_packet(u8 *adv,u8 len,CTRL_PACKAGE *buf);
void 	BLE_event_parser(void);
void 	BLE_SPI_process(void);
void  BLE_process(void);
void 	BLE_modify_UUID(u16 fun,u16 uuid);
void 	BLE_UUID_property(u16 fun,u16 property);

#endif   /* _BLEPROCESS_H_ */
