/* ************************************************* *
 * 			PIC24 Developer Kit Board Support        *
 * ************************************************* */

#ifndef _PIC24_DEVKIT_H
#define _PIC24_DEVKIT_H

/* LED ports */
#define mLED1					LATFbits.LATF4
#define mLED2					LATGbits.LATG6
#define mLED3					LATGbits.LATG8

#define mInitLED()				TRISFbits.TRISF4 = 0; TRISGbits.TRISG6 = 0, TRISGbits.TRISG8 = 0

#endif
