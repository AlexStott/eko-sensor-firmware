#define TARG_PIC24F_SK
#define SBBR1PDC1

#include "HardwareProfiles.h"
#include "modbus.h"
#include <uart.h>

/* Set config bits programatically. Helper macros defined in HW Profiles file */
mSetConfig2Defs()
mSetConfig1Defs()

slaveStateType state; // enum type for state
pduType rx_pdu; // pdu structs
pduType tx_pdu;

#define SENSOR_ID 0x11

unsigned char rx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)
unsigned char tx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)
unsigned short firstR = 0;
unsigned short numofR = 0;




//main loop
int main(void)
{
	// enum pduErrorType result;
    unsigned int len = 0;
	
	// Set up Clock
	OSCCON	=	0x11C0;	 //select INTERNAL RC, Post Scale PPL (fixme)
	
	// moveme
	#ifdef __PIC24F256GB106__
	RPINR18bits.U1RXR = 3;
	RPOR2bits.RP4R = 3;
	
	TRISFbits.TRISF4 = 0;
	TRISGbits.TRISG6 = 0;
	TRISGbits.TRISG8 = 0;
	#endif
	
	// fixme
	T1CON = 0x8030;
	
	// Setup UART
	U1MODE = 0x8000; // UART Enable
	U1STA = 0x0400; // TX Enable
	U1BRG = 25;	// 9600 Baud
	
	// Clear UART Interrupts
	IFS0bits.U1RXIF = 0;
    IFS0bits.U1TXIF = 0;

	//ConfigIntUART1(UART_RX_INT_EN & UART_TX_INT_EN);
	while (1)
	{
	
		while(BusyUART1());
		
		len = modbusRecvLoop();
		checkPDU(len, &rx_pdu);
		//processPDU(tx_pdu, rx_pdu);
		//formatPDU(tx_pdu, rx_pdu);
		//state = transmitPDU(void);
		LATFbits.LATF4 = 0;
		LATGbits.LATG6 = 0;
		LATGbits.LATG8 = 1;
	}
}


