
/*
Engscope Tutorial
JL
May 6, 2009
ADC module PIC24F
*/

#include "P24FJ256GB106.h"
#include "adc.h"

#define ADC_VOLTAGE 5	//select input
#define ADC_VOLTAGE2 3	//select input

//declare variables
unsigned int voltage;

//init function
void ADCInit()
{
   //Turn on, auto sample start, auto-convert
   AD1CON1 = 0x0000;

   AD1CON3 = 0x0000;

   AD1CHS = 0x0005;
   AD1PCFGbits.PCFG5 = 0;  //Disable digital input on AN5
   AD1PCFGbits.PCFG3 = 0;
   AD1CSSL = 0;	 //No scanned inputs
   //Vref+, Vref-, int every conversion, MUXA only
   AD1CON2 = 0x0000;

}

//main process function
int ADCProcessEventsA()
{
   
while (!AD1CON1bits.DONE);
   voltage = ADC1BUF0;
return voltage;
}

int ADCProcessEventsB()
{
   
while (!AD1CON1bits.DONE);
   voltage = ADC1BUF1;
return voltage;
}