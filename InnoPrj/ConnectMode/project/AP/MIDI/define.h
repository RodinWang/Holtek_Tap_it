/**********************************************
文件名称：	define.h
功能描述：
**********************************************/
//此文件用来定义常量
#ifndef	__DEFINE_H
	#define	__DEFINE_H
	#define	c_addr_tone_def		0		//音色头地址
	#define	c_addr_flash_offset	0		//0x9600/2 //编译COD 偏移0x9600
	
	#define	c_tone_perc			128
	#define	c_pan_init			0
	
	
	
	#define	c_trans_max 		6
	#define	c_trans_min 		250
	#define	c_trans_init 		0

	#define	c_vol_max			63
	#define	c_channel_min			1	//channel 从 1开始  channel 0用于读取资料
	
	
	#define	c_time_init_delay_demo_replay	0
	#define	c_time_min_delay_demo_replay	1
	#define	c_time_max_delay_demo_replay	500
	
	#define	c_addr_table_midi			0x200
	#define	c_unit_addr_midi			0x80	//midi地址最小单位 128个字

	#define	c_u8_true				0xff
	#define	c_u8_false			0
	#define	c_flag_true			1
	#define	c_flag_false		0

	#define	c_notekey_play			0x80
	#define	c_midi_play					0x40
	#define	c_channel_no_play		0
	#define c_u8_value_null			0xff		//无效值定义
	
	
//#define IS_VIB(level) ((level >=0) && (level <=4))
#define IS_VIB(x) ((x ==0) || (x ==1) || (x ==2)  || (x ==3) || (x ==4))

//u8	R_Midi_Track_Para[16*4];//tone value volume pan
//#define	c_track_tone			0
#define	c_track_value				1
#define	c_track_volume			2
//#define	c_track_pan				3

#define GPIO_PA         						(0)

#define KEY1_BUTTON_GPIO_ID         (GPIO_PA)
#define KEY1_BUTTON_GPIO_PIN        (GPIO_PIN_0)
#define KEY1_BUTTON_AFIO_MODE       (AFIO_FUN_GPIO)
#define KEY1_BUTTON_EXTI_CHANNEL    (0)

#define KEY2_BUTTON_GPIO_ID         (GPIO_PA)
#define KEY2_BUTTON_GPIO_PIN        (GPIO_PIN_1)
#define KEY2_BUTTON_AFIO_MODE       (AFIO_FUN_GPIO)
#define KEY2_BUTTON_EXTI_CHANNEL    (1)
	
#define KEY3_BUTTON_GPIO_ID         (GPIO_PA)
#define KEY3_BUTTON_GPIO_PIN        (GPIO_PIN_2)
#define KEY3_BUTTON_AFIO_MODE       (AFIO_FUN_GPIO)
#define KEY3_BUTTON_EXTI_CHANNEL    (2)

#define KEY4_BUTTON_GPIO_ID         (GPIO_PA)
#define KEY4_BUTTON_GPIO_PIN        (GPIO_PIN_3)
#define KEY4_BUTTON_AFIO_MODE       (AFIO_FUN_GPIO)
#define KEY4_BUTTON_EXTI_CHANNEL    (3)

#define KEY5_BUTTON_GPIO_ID         (GPIO_PA)
#define KEY5_BUTTON_GPIO_PIN        (GPIO_PIN_4)
#define KEY5_BUTTON_AFIO_MODE       (AFIO_FUN_GPIO)
#define KEY5_BUTTON_EXTI_CHANNEL    (4)

#define KEY6_BUTTON_GPIO_ID         (GPIO_PA)
#define KEY6_BUTTON_GPIO_PIN        (GPIO_PIN_5)
#define KEY6_BUTTON_AFIO_MODE       (AFIO_FUN_GPIO)
#define KEY6_BUTTON_EXTI_CHANNEL    (5)

#define KEY7_BUTTON_GPIO_ID         (GPIO_PA)
#define KEY7_BUTTON_GPIO_PIN        (GPIO_PIN_6)
#define KEY7_BUTTON_AFIO_MODE       (AFIO_FUN_GPIO)
#define KEY7_BUTTON_EXTI_CHANNEL    (6)

#define KEY8_BUTTON_GPIO_ID         (GPIO_PA)
#define KEY8_BUTTON_GPIO_PIN        (GPIO_PIN_7)
#define KEY8_BUTTON_AFIO_MODE       (AFIO_FUN_GPIO)
#define KEY8_BUTTON_EXTI_CHANNEL    (7)
#endif
