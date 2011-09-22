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
	
	#define CFG_EE_MBADDR  		0x00 /* Modbus Address */
	#define CFG_EE_MBCLS   		0x01 /* Modbus Device Class */
	#define CFG_EE_ADC_ISEL  	0x02 /* ADC Input Select (Hi Byte) */
	#define CFG_EE_ADC_REPT  	0x03 /* ADC Repeat Count */
	#define CFG_EE_ADC_SAMP  	0x04 /* ADC Sample Interval (time between sampl sequences*/
	#define CFG_EE_ADC_WAIT  	0x05 /* ADC WSait between two measurements */
	
	#define CFG_EE_I2C_CHIP  	0x06 /* I2C Device Enable Register */
	
	/* Unused EEPROM Data */

	#define CFG_EE_CRC_LO  0x0E /* Config Section CRC */
	#define CFG_EE_CRC_HI  0x0F
	#define CFG_EE_LOCK_HI 0x10
	#define CFG_EE_LOCK_LO 0x11
#endif
	/* Runtime configuration memory */
	#define CFG_ADC_CONTROL		0x12 // 0x8009
	#define CFG_I2C_CONTROL		0x13
	
	#define	DAT_I2C_TEMP_HI		0x14 // 0x800A
	#define DAT_I2C_TEMP_LO		0x15
	
	#define DAT_I2C_LIGHT_HI	0x16 // 0x800B
	#define DAT_I2C_LIGHT_LO	0x17

	#define SYS_MB_MSG_COUNT	0x19 // 0x800C
	#define SYS_MB_ERR_COUNT	0x1B // 0x800D
	#define SYS_FW_VERSION		0x1C // 0x800E
	#define SYS_HW_VERSION		0x1D
	
	
#endif
