/* ********************************************************************* *
 * 				Energy Kiosk Observer : Sensor Firmware					 *
 * 						Version 2 / Aug 2011							 *
 *  																	 *
 * ********************************************************************* *
 * 		AUTHORS:		Charith Amarasinghe <ca508@ic.ac.uk>			 *
 *						Gonzalo de Gisbert	<gdg90@ic.ac.uk>	         *
 *																		 *
 * ********************************************************************* *
 */
#define USE_AND_OR
#define TARGET_EKOBB_R3
#include "board/chip.h"
#include "board/p24_fuses.h"
#include "board/p24_board.h"

//p24 libs
#include <uart.h>
#include <timer.h>

//mb crc tables
#include "include/crc_tables.h"

unsigned char rxbuf[64];
unsigned char txbuf[64];
unsigned char rx_msg_len;
unsigned char error_code;

void init_uart1(int BRG)
{
	U1MODE = 0x8B00; /* Enable UART, set RTS to simplex */
	U1STA = 0x0400; /* Enable transmit */
	U1BRG = BRG;
}

void read_uart1_pdu( void )
{
	rx_msg_len = 0; // reset index to zero

	/* we use TIMER2 to detect a 4ms (3.5char 9600bps) interval after frame */
	TMR2 = 0x0000;
	T2CON = 0x8030; /* enable timer 2 with 1:256 prescale */
	/* at 8Mhz Fosc, timer tick is 0.064ms, so 4ms = 63 */

	while (T2CON < 63)
	{
		if (U1STAbits.URXDA)
		{
			rxbuf[rx_msg_len++] = (unsigned char)(0x00FF & U1RXREG);
			TMR2 = 0x0000;
			mLED1 = U1STAbits.OERR;
		}
		if (rx_msg_len > 63) return;
	}
	
	//msg in buffer, rx_msg_len is up to date
	return;
}

void write_uart1_pdu( int pduLen )
{
	while (pduLen != 0)
	{
		pduLen--;
		while (U1STAbits.UTXBF);
		U1TXREG = (0x00FF & txbuf[pduLen]);
	}
	return;
}

int main (void)
{
	init_uart1(25);
	mInitLED();
	TRISBbits.TRISB2 = 1; // In
	TRISBbits.TRISB7 = 0; // Out
	TRISBbits.TRISB8 = 0;
	while(1)
	{

		if (U1STAbits.URXDA)
		{
			read_uart1_pdu();
			mLED2_On();
		}
	}
}

