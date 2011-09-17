#ifndef _P24_BOARD_EKOBBR2_H
#define _P24_BOARD_EKOBBR2_H

#define mLED1					LATAbits.LATA3
#define mLED2					LATAbits.LATA2
#define mLED3					LATBbits.LATB14 // Not populated
#define mInitLED()				TRISAbits.TRISA2 = 0; TRISAbits.TRISA3 = 0;

#define UART1_RTS				LATBbits.LATB8
#define BUS_INTERRUPT			LATAbits.LATA2

#define BUSTX					0
#define BUSRX					1

#define RESET_EEPROM			PORTBbits.RB15
#endif
