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

/* I2C EEPROM can configure device functionality */
#ifdef EN_CFG_EEPROM
	#define EE_ADDR_CFG	   0xA0 /* Base address for EEPROM */
	
	#define CFG_EE_MBADDR  0x00 /* Modbus Address */
	#define CFG_EE_MBCLS   0x01 /* Modbus Device Class */
	#define CFG_EE_ADC_SR  0x02 /* Modbus ADC Setting Register */
	#define CFG_EE_ADC_R0  0x03 /* Modbus ADC Measurement 1 Register */
	#define CFG_EE_ADC_R1  0x04 /* Modbus ADC Measurement 2 Register */
	#define CFG_EE_ADC_R2  0x05 /* ... */
	#define CFG_EE_ADC_R3  0x06 /* ... */
	#define CFG_EE_ADC_R4  0x07 /* ... */
	
	#define CFG_EE_I2C_SR  0x08 /* Modbus I2C Device Setting Register */
	#define CFG_EE_CRC_LO  0x09 /* Config Section CRC */
	#define CFG_EE_CRC_HI  0x0A
#endif


#endif
