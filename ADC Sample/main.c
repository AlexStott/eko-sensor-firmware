
/*
Engscope Tutorial
JL
May 6, 2009
ADC module PIC24F
*/

#define USE_AND_OR
#include "P24FJ256GB106.h"
#include "adc.h"
#include <timer.h>


_CONFIG2(IESO_OFF  & FNOSC_FRC & FCKSM_CSDCMD  & IOL1WAY_OFF) 
_CONFIG1(JTAGEN_OFF & ICS_PGx2 & FWDTEN_OFF)

#define dataLength 32
unsigned int	data[dataLength];
long dataV[dataLength];

//main loop
int main(void)
{
   //select Primary Oscillator, External XL, PPL
 	OSCCON	=	0x11C0;	
	T1CON = 0x0030;

	int i,k = 0;
  	ADCInit();
	//ADC1BUF0 = 0x0000;
	AD1CON1bits.ADON = 1;
	
	while(1)
	{
	T1CONbits.TON = 1;

    for (i=0;i<dataLength-1;i=i+2)
	{	
      	
		TMR1=0;
		AD1CON1bits.SAMP = 1;
		for (k=0;k<5;k++);
		AD1CON1bits.SAMP = 0;
		data[i] = ADCProcessEventsA();

		for(k=0;k<100;k++)

		AD1CHS = 0x0003;
		AD1CON1bits.SAMP = 1;
		for (k=0;k<5;k++);
		AD1CON1bits.SAMP = 0;
		data[i+1] = ADCProcessEventsA();
		AD1CHS = 0x0005;
		//AD1CON1bits.SAMP = 1;
		//for (k=0;k<5;k++);
		//AD1CON1bits.SAMP = 0;
		//data[i+1] = ADCProcessEventsA();
		while (TMR1 <= 40);
		

	}

	T1CONbits.TON = 0;
	
	for (i=0;i<dataLength;i++)
	{
		dataV[i] = (long)data[i];
		dataV[i] = (dataV[i] * 3300) / 1024;
	}
	//for (i=0;i<dataLength;i++)
	//{
	//	dataI[i] = (long)data[i+1];
	//	dataI[i] = (dataV[i+1] * 3300) / 1024;   //what value for normalising current? 3300?!
	//}
	for (i=0;i<2;i++);   
}

}