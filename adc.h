
//define Voltage & Current Input Pins
#define ADC_V 			5	//select Voltage input
#define ADC_I 			3	//select Current input

//define how many samples required
#define dataLength 	32

//data buffers variables
extern unsigned int	Vbuff[dataLength];
extern unsigned int	Ibuff[dataLength];

//functions
void ADCInit(void);
int ADCProcessEvents(void);
void ADCmain(void);