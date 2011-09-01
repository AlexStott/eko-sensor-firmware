#ifndef _P24_BOARD_EKOBBR3_H
#define _P24_BOARD_EKOBBR3_H

#define mLED1					LATBbits.LATB4 // frame LED
#define mLED2					LATAbits.LATA4 // error LED
#define mLED3					LATBbits.LATB14 // Not populated
#define mInitLED()				TRISBbits.TRISB4 = 0; TRISAbits.TRISA4 = 0;

#endif
