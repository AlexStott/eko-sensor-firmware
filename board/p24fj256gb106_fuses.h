/* ************************************************* *
 * 			PIC24 Developer Kit Fuses                *
 * ************************************************* */

#ifndef _P24FJ256GB106_FUSES_H
#define _P24FJ256GB106_FUSES_H



/* Configuration for the PIC24F starter kit development environment */
#ifndef __PIC24FJ256GB106__
	#error "Check MCU Selection"
#endif


#include <p24fj256gb106.h>


/* Configuration Bit defaults for platform */
_CONFIG2(IESO_OFF  & FNOSC_FRC & FCKSM_CSDCMD  & IOL1WAY_OFF) 
// Two Speed Startup off, FRC Osc with No Post-Scale, PLL off, RP registers locked
_CONFIG1(JTAGEN_OFF & ICS_PGx2 & FWDTEN_OFF)
// JTAG off, PG2 pins for ICSP, Watchdog Timer off


#endif

