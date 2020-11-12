
#ifndef _MY_CONST
#define _MY_CONST
#include "ht32.h"
//===================================================
//=============   Queue data struct   ===============
//===================================================

#define QUEUE_MAX 7

typedef struct _QueueElement
{
	u8 EID;
	u8 Tapped;
	u32 BeatTime;
}QueueElement;
	


//===================================================
//=============     Normal const     ================
//===================================================

#define GPIO_PORT_FLEFT 	HT_GPIOB
#define GPIO_PORT_FRIGHT 	HT_GPIOB
#define GPIO_PORT_MODE_SW HT_GPIOC


#define GPIO_PIN_FLEFT 		GPIO_PIN_7
#define GPIO_PIN_FRIGHT 	GPIO_PIN_8
#define GPIO_PIN_MODE_SW_1 GPIO_PIN_14
#define GPIO_PIN_MODE_SW_2 GPIO_PIN_15

#define MAX_LED_CNT	6
#define MAX_GPIO_CNT 2

//===================================================
//===================================================
//=============       ID const     ==================
//===================================================
//===================================================

// LED ring EID
#define EID_LED_1 	0x01
#define EID_LED_2 	0x02
#define EID_LED_3 	0x03
#define EID_LED_4 	0x04
#define EID_LED_5 	0x05
#define EID_LED_6 	0x06


// Vibrate detection EID
#define EID_VIBR_1 	0x01
#define EID_VIBR_2 	0x02
#define EID_VIBR_3 	0x03
#define EID_VIBR_4 	0x04



//===================================================
//===================================================
//=============       Color const     ===============
//===================================================
//===================================================

// color brightness
#define COLOR_BRIGHTNESS 	0x20

// generate color code function
#define COLOR_CODE_GEN(R, G, B)  R, G, B, R, G, B, R, G, B, R, G, B,\
								 R, G, B, R, G, B, R, G, B, R, G, B,\
								 R, G, B, R, G, B, R, G, B, R, G, B,\
								 R, G, B, R, G, B, R, G, B, R, G, B

//=========================  R     G     B
// Color code red
#define COLOR_CODE_DARKRED 			0x8B, 0x00, 0x00

// Color code red
#define COLOR_CODE_RED 					0xFF, 0x00, 0x00

// Color code orange
#define COLOR_CODE_ORANGE				0xFF, 0x28, 0x00

// Color code yellow
#define COLOR_CODE_YELLOW				0xFF, 0x6E, 0x00

// Color code light green
#define COLOR_CODE_LIGHTGREEN		0x48, 0xEE, 0x28

// Color code green
#define COLOR_CODE_GREEN 				0x00, 0x80, 0x00

// Color code light blue
#define COLOR_CODE_LIGHTBLUE		0x78, 0xB4, 0xE6

// Color code blue
#define COLOR_CODE_BLUE 				0x00, 0x00, 0xFF

// Color code purple
#define COLOR_CODE_PURPLE				0x80, 0x00, 0x80

// Color code black
#define COLOR_CODE_BLACK				0x00, 0x00, 0x00


//===================================================
//===================================================
//=============       LED const     =================
//===================================================
//===================================================

#define LED_DIRECTION_OPEN 0x00
#define LED_DIRECTION_CLOSE 0x01

#define LED_TIMEOUT_TIMING 80
#define LED_END_TIMING 80
#define LED_ANIMATION_TIMING 8

#define LED_DEFAULT_HOLD_TIME 10
#define LED_END_HOLD_TIME 10

#define LED_ROTATE_SINGLE 1
#define LED_ROTATE_MULTI 0

//===================================================
//===================================================
//=============      Song const     =================
//===================================================
//===================================================
// audio type
#define AUDIO_TYPE_MIDI 1
#define AUDIO_TYPE_VOICE 2

// language
#define LANG_COUNT 2

#define LANG_ENG 1
#define LANG_ZH_TW 2

// sub id introduction voice
#define SUB_ID_IDLE_MODE 0
#define SUB_ID_TAP_MODE 1
#define SUB_ID_CONNECTING_BT 2
#define SUB_ID_BT_CONNECTED 3
#define SUB_ID_SONG_MODE 4
#define SUB_ID_GAME_START 5
#define SUB_ID_COUNT 6

// Song start id 
#define SONG_BASE (LANG_COUNT*SUB_ID_COUNT)

// Midi Song
#define SONG_LITTLE_STAR 1
#define SONG_CANON 2
#define SONG_DEMONS 3

#define GEN_SONG_ID_SUB(SUB_ID,LANG) (LANG_COUNT*SUB_ID + LANG)
#define GEN_SONG_ID_VOICE(SONG) (SONG+SONG_BASE)
#define GEN_SONG_ID_MIDI(SONG) (SONG)

#define SONG_ID_IDLE_MODE_ENG 			GEN_SONG_ID_SUB(SUB_ID_IDLE_MODE, LANG_ENG)
#define SONG_ID_IDLE_MODE_ZH_TW 		GEN_SONG_ID_SUB(SUB_ID_IDLE_MODE, LANG_ZH_TW)
#define SONG_ID_TAP_MODE_ENG				GEN_SONG_ID_SUB(SUB_ID_TAP_MODE, LANG_ENG)
#define SONG_ID_TAP_MODE_ZH_TW			GEN_SONG_ID_SUB(SUB_ID_TAP_MODE, LANG_ZH_TW)
#define SONG_ID_CONNECTING_BT_ENG		GEN_SONG_ID_SUB(SUB_ID_CONNECTING_BT, LANG_ENG)
#define SONG_ID_CONNECTING_BT_ZH_TW	GEN_SONG_ID_SUB(SUB_ID_CONNECTING_BT, LANG_ZH_TW)
#define SONG_ID_BT_CONNECTED_ENG		GEN_SONG_ID_SUB(SUB_ID_BT_CONNECTED, LANG_ENG)
#define SONG_ID_BT_CONNECTED_ZH_TW	GEN_SONG_ID_SUB(SUB_ID_BT_CONNECTED, LANG_ZH_TW)
#define SONG_ID_SONG_MODE_ENG				GEN_SONG_ID_SUB(SUB_ID_SONG_MODE, LANG_ENG)
#define SONG_ID_SONG_MODE_ZH_TW			GEN_SONG_ID_SUB(SUB_ID_SONG_MODE, LANG_ZH_TW)
#define SONG_ID_GAME_START_ENG			GEN_SONG_ID_SUB(SUB_ID_GAME_START, LANG_ENG)
#define SONG_ID_GAME_START_ZH_TW		GEN_SONG_ID_SUB(SUB_ID_GAME_START, LANG_ZH_TW)


#define SONG_TYPE_LITTLE_STAR	AUDIO_TYPE_MIDI
#define SONG_ID_LITTLE_STAR GEN_SONG_ID_MIDI(SONG_LITTLE_STAR)

#define SONG_TYPE_CANON AUDIO_TYPE_MIDI
#define SONG_ID_CANON GEN_SONG_ID_MIDI(SONG_CANON)

#define SONG_TYPE_DEMONS AUDIO_TYPE_MIDI
#define SONG_ID_DEMONS GEN_SONG_ID_MIDI(SONG_DEMONS)

#endif
