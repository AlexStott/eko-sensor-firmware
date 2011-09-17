/* ************************************************* *
 * 			PIC24F16KA102 Baseboard Fuses            *
 * ************************************************* */

#ifndef _P24F16KA102_FUSES_H
#define _P24F16KA102_FUSES_H

#ifndef __PIC24F16KA102__
	#error "Check MCU Selection"
#endif

#include <p24f16ka102.h>

/* Select the FRC oscillator, disable two speed startup */
_FOSCSEL(FNOSC_FRC & IESO_OFF)

/* Disable clock switching, enable clock monitor, low power secondary, OSCO pins for DIO */
_FOSC( FCKSM_CSDCMD & SOSCSEL_SOSCLP & OSCIOFNC_ON & POSCMOD_NONE)

/* Disable WDT. We may need this, but have yet to decide when to kick the dog */
_FWDT(FWDTEN_OFF & WINDIS_OFF & FWPSA_PR32 & WDTPS_PS1024)

/* Enable MCLR, BORV set to 2.7, Select aux I2C pins, PUT Enabled, BOR in H/W */
_FPOR(MCLRE_ON & BORV_V27 & I2C1SEL_SEC & PWRTEN_ON & BOREN_BOR3)

#if defined (TARGET_EKOBB_R3)
	//Fix CA 08/2011: BBR3 uses PGx2 pins for programming. Setting required for proper debug.
	_FICD(ICS_PGx2)
#endif

#if defined (TARGET_EKOBB_R2)
	_FICD(ICS_PGx1)
#endif

#endif
