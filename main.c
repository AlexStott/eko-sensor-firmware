#include "p24FJ256GB106.h"
#include <uart.h>
#include "modbus.h"
#include <timer.h>

#define USE_AND_OR 

_CONFIG2(IESO_OFF & FNOSC_FRC & POSCMOD_NONE) // Primary osc disabled, FRC OSC with PLL, USBPLL /2
_CONFIG1(JTAGEN_OFF & ICS_PGx2 & FWDTEN_OFF)        // JTAG off, watchdog timer off

enum slaveStateType state;
struct pduType rx_pdu;
struct pduType tx_pdu;

#define TIMER_3C5 2000
#define SENSOR_ID 0x11

unsigned char rx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)
unsigned char tx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)
unsigned short firstR = 0;
unsigned short numofR = 0;


unsigned char modbusRecvLoop(void);
void checkPDU(unsigned char pduLength, struct pduType* msg);

//main loop
int main(void)
{
	//enum pduErrorType result;
    unsigned char len = 0;
	
	//Set up Clock
	OSCCON	=	0x11C0;	 //select INTERNAL RC, Post Scale PPL
	//Loop forever
	RPINR18bits.U1RXR = 3;
	RPOR2bits.RP4R = 3;
	T1CON = 0x8030;
	OpenUART1(UART_EN, UART_TX_ENABLE, 25);
	ConfigIntUART1(UART_RX_INT_EN & UART_TX_INT_EN);
	while (1)
	{
	TRISFbits.TRISF4 = 0;
	TRISGbits.TRISG6 = 0;
	TRISGbits.TRISG8 = 0;
	U1BRG = 25;	//set baud speed
   	U1MODE	=	0x8000;	 //turn on module
    //U1STA	=	0x8400;	 //set interrupts  
	
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

unsigned char modbusRecvLoop(void)
{
  // initialise everything
  unsigned char index = 0;
  state = SLAVE_IDLE; 
  while ((state == SLAVE_IDLE) || ((state == SLAVE_RECEIVING) && (TMR1 < TIMER_3C5 )))
  {
    if (DataRdyUART1())
    {
	state = SLAVE_RECEIVING;
	LATFbits.LATF4 = 0;
	LATGbits.LATG6 = 1;
	LATGbits.LATG8 = 0;
    TMR1 = 0;
    rx_buffer[index++] = ReadUART1();
    
      
    }
  }

  return index;
}

// process received data buffer
void checkPDU(unsigned char pduLength, struct pduType* msg)
{
  int x;
  state = SLAVE_CHECKING;
  if (rx_buffer[0] != SENSOR_ID)
    //return NOT_ADDR;
	{
	LATFbits.LATF4 = 1;
	LATGbits.LATG6 = 1;
	LATGbits.LATG8 = 1;
	while (TMR1 < 20000);
	return; 
	}
  /* Insert CRC checking code here. See PIC24F peripheral library */
  //if (!checkCRC)
   // return CRC_FAIL;
  // fill struct
  msg->address = rx_buffer[0];
  msg->function = rx_buffer[1];
  for (x=0;x<pduLength-4;x++)
  {
    // smart thing to do: make msg->data ptr, and set to address of &rx_buffer[2]
    msg->data[x] = rx_buffer[x+2];
	
  }
  msg->data_length = pduLength-4; 
	LATFbits.LATF4 = 1;
	LATGbits.LATG6 = 0;
	LATGbits.LATG8 = 0;
	while (TMR1 < 20000);  
  //return MSG_OK;
return;
}
