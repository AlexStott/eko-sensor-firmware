#ifndef _P24_BOARD_EKOBBR3_H
#define _P24_BOARD_EKOBBR3_H

#define mLED1					LATBbits.LATB4
#define mLED2					LATAbits.LATA4
#define mLED3					LATBbits.LATB14 // Not populated
#define mInitLED()				TRISBbits.TRISB4 = 0; TRISAbits.TRISA4 = 0;

#endif
