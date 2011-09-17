#ifndef _P24_BOARD_EKOBBR3_H
#define _P24_BOARD_EKOBBR3_H

#define mLED1					LATBbits.LATB4 // frame LED
#define mLED2					LATAbits.LATA4 // error LED
#define mLED3					LATBbits.LATB14 // Not populated
#define mInitLED()				TRISBbits.TRISB4 = 0; TRISAbits.TRISA4 = 0;

#define UART1_RTS				LATBbits.LATB8
#define BUS_INTERRUPT			LATAbits.LATA2

#define BUSTX					1
#define BUSRX					0

#define RESET_EEPROM			PORTBbits.RB15
#endif
