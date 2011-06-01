#include "p24FJ256GB106.h"
#include <uart.h>
#include "modbus.h"
#include<timer.h>

_CONFIG2(IESO_OFF & FNOSC_FRC & POSCMOD_NONE) // Primary osc disabled, FRC OSC with PLL, USBPLL /2
_CONFIG1(JTAGEN_OFF & ICS_PGx2 & FWDTEN_OFF)        // JTAG off, watchdog timer off

enum slaveStateType state;
struct pduType rx_pdu;
struct pduType tx_pdu;

#define TIMER_3C5 1458
#define SENSOR_ID 0x11

unsigned char rx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)
unsigned char tx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)
unsigned short firstR = 0;
unsigned short numofR = 0;


unsigned char modbusRecvLoop(void);

//main loop
int main(void)
{
	//enum pduErrorType result;
    unsigned char len;
	
	//Set up Clock
	OSCCON	=	0x11C0;	 //select INTERNAL RC, Post Scale PPL
	//Loop forever
	RPINR18bits.U1RXR = 3;
	RPOR2bits.RP4R = 3;
	
	OpenUART1(UART_EN, UART_TX_ENABLE, 25);
	//ConfigIntUART1(UART_RX_INT_EN & UART_TX_INT_EN);
	while (1)
	{
	U1BRG = 25;	//set baud speed
    U1MODE	=	0x8000;	 //turn on module
    U1STA	=	0x8400;	 //set interrupts
	while(BusyUART1());
	
	len = modbusRecvLoop();
	//result = checkPDU(len, rx_pdu);
	//processPDU(tx_pdu, rx_pdu);
	//formatPDU(tx_pdu, rx_pdu);
	//state = transmitPDU(void);
	}
}

unsigned char modbusRecvLoop(void)
{
  // initialise everything
  unsigned char index = 0;
unsigned int x = 0;
  state = SLAVE_IDLE;
  OpenTimer1(T1_OFF, 145800000); // open the timer but turn it off and reset to 0
  // block 
  while ((state == SLAVE_IDLE) || ((state == SLAVE_RECEIVING) && (ReadTimer1() != 0)))
  {
    if (DataRdyUART1())
    {

      OpenTimer1(T1_ON, 145800000);
	x=ReadTimer1();
      rx_buffer[index++] = ReadUART1();
      state = SLAVE_RECEIVING;
      
    }
  }
  
  CloseTimer1();
  return index;
}