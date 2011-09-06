
//define Voltage & Current Input Pins
#define VSAMP_CH 			0x0000	//select Voltage input
#define ISAMP_CH		0x0005	//select Current input

// buffer indices for voltage and current samples
#define	VBUFF_IDX	0
#define	IBUFF_IDX   1


//define how many samples required
#define dataLength 	32

//data buffers variables
extern unsigned int	Vbuff[dataLength];
extern unsigned int	Ibuff[dataLength];
extern unsigned int analog_buffer[3][32];
//functions
void ADCInit(void);
int ADCProcessEvents(void);
void ADCmain(void);

void sample_adc_channel(unsigned int channel, unsigned char buf_num, unsigned int bufnum_idx, unsigned char sh_cycles);
