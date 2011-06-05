//Hardware Specific Configuration for EKO sensor baseboards / development tools
#ifndef _HARDWARE_PROFILE_H
#define _HARDWARE_PROFILE_H
	
	#if defined (TARG_PIC24F_SK)
		/* Configuration for the PIC24F starter kit development environment */
		#ifndef __PIC24FJ256GB106__
			#error "Check MCU Selection"
		#endif
		
		#include "p24FJ256GB106.h"
		

		
		/* Configuration Bit defaults for platform */
		_CONFIG2(IESO_OFF  & FNOSC_FRC & FCKSM_CSDCMD  & IOL1WAY_OFF) 
		// Two Speed Startup off, FRC Osc with No Post-Scale, PLL off, RP registers locked
		_CONFIG1(JTAGEN_OFF & ICS_PGx2 & FWDTEN_OFF)
		// JTAG off, PG2 pins for ICSP, Watchdog Timer off
		
		/* LED ports */
		#define mLED1					LATFbits.LATF4
		#define mLED2					LATGbits.LATG6
		#define mLED3					LATGbits.LATG8
		

		
	#elif defined (TARG_EKOBB_R1)
		#ifndef __PIC24F16KA102__
			#error "Check MCU Selection"
		#endif
		
		#include "p24F16KA102.h"
		

		
		/* Select the FRC oscillator, disable two speed startup */
		_FOSCSEL(FNOSC_FRC & IESO_OFF)
		
		/* Disable clock switching, enable clock monitor, low power secondary, OSCO pins for DIO */
		_FOSC( FCKSM_CSECME & SOSCSEL_SOSCLP & OSCIOFNC_OFF & POSCMOD_NONE)
		
		/* Disable WDT. We may need this, but have yet to decide when to kick the dog */
		_FWDT(FWDTEN_OFF & WINDIS_OFF)
		
		/* Enable MCLR, BORV set to 2.7, Select aux I2C pins, PUT Enabled, BOR in H/W */
		_FPOR(MCLRE_ON & BORV_V27 & I2C1SEL_SEC & PWRTEN_ON & BOREN_BOR0)
		
		#define mLED1					LATAbits.LATA2
		#define mLED2					LATAbits.LATA3
		#define mLED3					LATBbits.LATB14 // Not populated
	#else
		#error "Define Target Platform"
	#endif
	
	/* LED On/Off Macros */
	/* Some platforms may not offer LED3 */
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
