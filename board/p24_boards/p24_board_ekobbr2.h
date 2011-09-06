#ifndef _P24_BOARD_EKOBBR2_H
#define _P24_BOARD_EKOBBR2_H

#define mLED1					LATAbits.LATA2
#define mLED2					LATAbits.LATA3
#define mLED3					LATBbits.LATB14 // Not populated
#define mInitLED()				TRISAbits.TRISA2 = 0; TRISAbits.TRISA3 = 0;

#endif
