#include "board/chip.h"
#include "board/p24_board.h"
#include "include/tmr2delay.h"

extern unsigned int databuf[];

void SampleAnalogPin( unsigned char channel, unsigned char dataloc );
void InitADC( void );



void InitADC( void )
{
	//ADC off. Set auto conversion start on internal count.
	AD1CON1 = 0x00E0;
	
	//AVDD, AVSS Reference, No Scan, No Alt
	AD1CON2 = 0x0000;
	
	//sys clock, Aquisition Time = 10TAD ~ 5us, TAD = 2.TCY
	AD1CON3 = 0x0500;
	
	AD1CSSL = 0;
	
}

// NB Func takes nearly 10us
void SampleAnalogPin( unsigned char channel, unsigned char dataloc )
{
	// ADC should be on
	
	// set channel
	AD1CHS = (unsigned int)channel & 0x000F;
	delay_us(1); // cap should discharge in about 0.5us
	
	AD1CON1bits.SAMP = 1;
	while (!AD1CON1bits.DONE);
	databuf[dataloc] = ADC1BUF0;
	
	return;
}

void ProcessADCEvents( unsigned char pin_mask, unsigned int pin_interval_us, unsigned char repeat_count, unsigned int repeat_interval_ms)
{
	unsigned char buf_offset = 0;
	unsigned char i = 0;
	mLED1 = 1;
	InitADC();
	
	AD1CON1bits.ADON = 1;
	delay_ms(6); // wait for the ADC to settle
	
	while (repeat_count--)
	{
		if (pin_mask & 0x01)
		{
			// AN0
			buf_offset = 0;
			SampleAnalogPin( 0, buf_offset+i );
			delay_us(pin_interval_us);
		}
		if ((pin_mask & 0x02) >> 1)
		{
			// AN1
			buf_offset = 32;
			SampleAnalogPin( 1, buf_offset+i );
			delay_us(pin_interval_us);
		}
		if ((pin_mask & 0x04) >> 2)
		{
			// AN2
			buf_offset = 64;
			SampleAnalogPin( 2, buf_offset+i );
			delay_us(pin_interval_us);
		}
		if ((pin_mask & 0x08) >> 3)
		{
			// AN3
			buf_offset = 96;
			SampleAnalogPin( 3, buf_offset+i );
			delay_us(pin_interval_us);
		}
		if ((pin_mask & 0x20) >> 5)
		{
			// AN5
			buf_offset = 128;
			SampleAnalogPin( 5, buf_offset+i );
			delay_us(pin_interval_us);
		}
		
		i++;
		// wait for next repetition
		delay_ms(repeat_interval_ms);
	}
	
	AD1CON1bits.ADON = 0;
	mLED1 = 0;
	return;
}
