#ifndef _P24_BOARD_H
#define _P24_BOARD_H

#include "chip.h"

#if (defined (TARGET_EKOBB_R1) || defined (TARGET_EKOBB_R2))

	#include "p24_boards\p24_board_ekobbr2.h"

#elif (defined (TARGET_EKOBB_R3))

	#include "p24_boards\p24_board_ekobbr3.h"

#elif (defined (TARGET_P24DEVKIT_SURE))

	#include "p24_boards\p24_board_devkit.h"

#endif

#define mLED1_On()				mLED1 = 1;
#define mLED2_On()				mLED2 = 1;
#define mLED3_On()				mLED3 = 1;

#define mLED1_Off()				mLED1 = 0;
#define mLED2_Off()				mLED2 = 0;
#define	mLED3_Off()				mLED3 = 0;

#define mLED1_Toggle()			mLED1 = !mLED1;
#define mLED2_Toggle()			mLED2 = !mLED2;
#define mLED3_Toggle()			mLED3 = !mLED3;

#endif
