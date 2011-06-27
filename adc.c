#include "global.h"
#include "SensorProfiles.h"
#include "modbus.h"
#include "adc.h"
#include "p24F16KA102.h"




// allocate buffers for AN0, AN1, AN5
unsigned int	analog_buffer[3][32];


//declare variables
unsigned int data;

//init function
void ADCInit()
{
   //ADC off, auto sample disabled
   AD1CON1 = 0x0000;
	//Use system clock, set TAD = TCY
   AD1CON3 = 0x0000;
	//Set input channel
   AD1CHS = VSAMP_CH;
	//Set all inputs to analog
   AD1PCFG = 0x0000;  
   //No scanned inputs
   AD1CSSL = 0;
   
   //Vdd & Vss Vrefs, disable alternate sample mode
   AD1CON2 = 0x0000;

}

//main process function
int ADCProcessEvents()
{
//Wait till conversion done (12 TADs)
while (!AD1CON1bits.DONE);
   data = ADC1BUF0;
return data;
}


//main loop
void ADCmain()
{
  	//Set Timer 1 256 prescaler
	T1CON = 0x0030;

	int i,k = 0;
	data = 0;
	
	//Initialise ADC
  	ADCInit();
	//turn on ADC
	AD1CON1bits.ADON = 1;
	//turn on Timer1
	T1CONbits.TON = 1;

	for (i=0;i<dataLength;i++)
	{	
		
		TMR1=0;
		//VOLTAGE ADC
		//start sampling and wait for 4 instruction clock cycles.
		//AD1CON1bits.SAMP = 1;
		//for (k=0;k<5;k++);
		//stop sampling, conversion automatically starts
		//AD1CON1bits.SAMP = 0; 
		//analog_buffer[VBUFF_IDX][i] = ADCProcessEvents();
		
		sample_adc_channel(VSAMP_CH, VBUFF_IDX, i, 5);
		
		
		for(k=0;k<100;k++)
		
		//CURRENT ADC
		//switch input to current pin
		sample_adc_channel(ISAMP_CH, IBUFF_IDX, i, 5);
		
		//wait for a delay of 2.5ms before taking next sample
		while (TMR1 <= 40);
			

	}
	//turn off ADC and Timer1	
	AD1CON1bits.ADON = 0;
	T1CONbits.TON = 0;
		
	return;
}

void sample_adc_channel(unsigned int channel, unsigned char buf_num, unsigned int bufnum_idx, unsigned char sh_cycles)
{
		int k;
		AD1CHS = channel;
		AD1CON1bits.SAMP = 1;
		for (k=0;k<sh_cycles;k++);
		AD1CON1bits.SAMP = 0;
		analog_buffer[buf_num][bufnum_idx] = ADCProcessEvents();
}
