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
#include "include/modbus2.h"

//p24 libs
#include <uart.h>

#define FCY		(4000000)
#define DEV_ADDR 0x02
unsigned char rxbuf[32];
unsigned char txbuf[64];

unsigned char rx_len;


void __attribute__((interrupt, auto_psv)) _DefaultInterrupt(void)
{
  while (1)
  {
      Nop();
      Nop();
      Nop();
  }
}

void __attribute__((interrupt, auto_psv)) _OscillatorFail(void)
{
  while (1)
  {
      Nop();
      Nop();
      Nop();
  }
}
void __attribute__((interrupt, auto_psv)) _AddressError(void)
{
  while (1)
  {
      Nop();
      Nop();
      Nop();
  }
}
void __attribute__((interrupt, auto_psv)) _StackError(void)
{
  while (1)
  {
      Nop();
      Nop();
      Nop();
  }
}
void __attribute__((interrupt, auto_psv)) _MathError(void)
{
  while (1)
  {
      Nop();
      Nop();
      Nop();
  }
}

void init_uart1(int BRG)
{
	U1BRG = BRG;
	U1MODE = 0x8000; /* Enable UART */
	U1STA = 0x0400; /* Enable transmit */
}

void error_led_on( void )
{
	mLED2_On()
}


void delay_ms( unsigned char ms )
{
	unsigned int ticks;
	ticks = (ms*1000/64);
	T2CON = 0x0000;
	TMR2 = 0;
	T2CON = 0x8030;
	while (TMR2 < ticks );
}

void delay_us( unsigned char us )
{
	unsigned int ticks;
	ticks = (us/64);
	T2CON = 0x0000;
	TMR2 = 0;
	T2CON = 0x8030;
	while (TMR2 < (ticks) );
}



char get_msg( void )
{
	unsigned char idx = 0;
	unsigned char thresh = 8;
	unsigned char temp;
	while ( !U1STAbits.URXDA  ) {}
	
	TMR2 = 0;
	T2CON = 0x8030; // 1:256 prescale, enable tmr2
	
 	while ( (idx < thresh) && (TMR2 < 150) )
	{
		while(U1STAbits.URXDA)
		{
			rxbuf[idx++] = (unsigned char)(0x00FF & U1RXREG);
		}
		if ((idx == 7) && (rxbuf[2] == 0x10))
		{
			// case for function 16
			thresh = 9 + rxbuf[6];
		}
	}
	
	T2CON = 0x0000;

	while( U1STAbits.URXDA )
	{
		temp = (unsigned char)(0x00FF & U1RXREG); // dump extra characters
	}

	return idx;
}

int main (void)
{

	unsigned char temp = 0;
	unsigned char i;
	AD1PCFG = 0xFFFF;
	TRISBbits.TRISB2 = 1; // In
	TRISBbits.TRISB7 = 0; // Out
	TRISBbits.TRISB8 = 0;
	LATBbits.LATB8 = 0;
	

	mInitLED();
	



	while(1)
	{	
		// init uart and interrupts
		rx_len = 0;
		init_uart1(25);
		mLED1 = 0;
		mLED2 = 0;
  		
		rx_len = get_msg();
		// message recvd. in rxbuf.
		temp = validate_pdu(DEV_ADDR, rx_len, rxbuf);
		
		
		if (temp == 1)
		{
			mLED2 = 0; // reset error led
			// message is known good!
			mLED1 = 1; // frame led on
			temp = process_pdu(rxbuf, txbuf);
		
	
			/* Transmit Response */
			LATBbits.LATB8 = 1; // TXEN
	
			delay_ms(2);
	
			for(i=0; i<temp; i++)
			{
				while (U1STAbits.UTXBF);
				U1TXREG = txbuf[i] & 0xFF;
			}

			mLED1 = 0; // frame led off
		}
		delay_ms(5);

		LATBbits.LATB8 = 0;
		Nop();
		Nop();
	}
}

