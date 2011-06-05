#define USE_AND_OR

#include "global.h"
#include "HardwareProfiles.h"
#include "SensorProfiles.h"

#include "modbus.h"
#include <uart.h>
#include <timer.h>
#include <crc.h>


// Configs defined in HardwareProfiles.h

// MODBUS specific structs and states
slaveStateType 	mb_state; 				// enum type for state
pduType 			mb_req_pdu; 			// pdu holding struct
pduType 			mb_resp_pdu;


unsigned short int test_buf[3];

volatile unsigned char		mb_req_timeout = 0x00;
volatile unsigned char		mb_crc_ready = 0x00;


// Receive and Transmit Buffers. MAX_DATA_LENGTH is defined in Modbus.h
unsigned char rx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)
unsigned char tx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)

/* Some  ISRs */

//  Timeout
void __attribute__ ((interrupt,no_auto_psv)) _T1Interrupt (void)
{
   T1_Clear_Intr_Status_Bit;
	if (mb_state == SLAVE_RECEIVING)
		mb_req_timeout = 1;
	
}

// CRC Done
void __attribute__((interrupt,no_auto_psv)) _CRCInterrupt (void)
{
    CRC_Clear_Intr_Status_Bit;
    mb_crc_ready = 1;
}



/* MODBUS Message Timeout Timer */

// initialises Timer1 for a timeout count
void init_mb_timeout_timer(unsigned int timer_ticks)
{
	CloseTimer1();
	mb_req_timeout = 0;
	OpenTimer1(T1_OFF | T1_PS_1_256, timer_ticks);
	ConfigIntTimer1(T2_INT_ON|T2_INT_PRIOR_1);
}

// start (or reset) a timeout timer
void start_mb_timeout_timer()
{
	T1CONbits.TON = 1;
	TMR1 = 0x00;
}

// close a timer (disable interrupts, disable timer.
void close_mb_timeout_timer()
{
	IEC0bits.T1IE = 0;      /* Disable the Timer1 interrupt */
    T1CONbits.TON = 0;      /* Disable timer1 */
    IFS0bits.T1IF = 0;      /* Clear Timer interrupt flag */
	mb_req_timeout = 0;
}

/* MODBUS CRC calculation PIC24F H/W */
unsigned int calculate_crc16(unsigned char *buffer, unsigned int length)
{
	unsigned int poly, crc_config, result;
	test_buf[0] = 0x1103;
	test_buf[1] = 0x0000;
	test_buf[2] = 0x000A;
	CRC_Config_INTR(CRC_INT_ENABLE | CRC_INT_PRIOR_4);
	poly=0x8005;
    crc_config = CRC_IDLE_STOP | CRC_POLYNOMIAL_LEN16 |CRC_START_SERIAL_SHIFT;
    CRC_Config (poly , crc_config);
	result = CRC_Calc_ChecksumWord(&test_buf,3,0x0000);
	while (!mb_crc_ready);
	mb_crc_ready = 0;
	return result;
}

void init_status_leds()
{
	#ifdef TARG_PIC24F_SK
		TRISFbits.TRISF4 = 0;
		TRISGbits.TRISG6 = 0;
		TRISGbits.TRISG8 = 0;
	#endif
}

void status_leds_off()
{
	mLED1_Off()
	mLED2_Off()
	mLED3_Off()
}

void status_leds_frame_on()
{
	mLED1_On()
}

void status_leds_frame_off()
{
	mLED1_Off()
}

//main loop
int main(void)
{
	pduErrorType result;
    unsigned int len = 0;
	
	// Set up Clock
	OSCCON	=	0x11C0;	 //select INTERNAL RC, Post Scale PPL (fixme)
	
	#ifdef TARG_PIC24F_SK
		// Development Kit needs port mapping
		RPINR18bits.U1RXR = 3;
		RPOR2bits.RP4R = 3;
	#endif
	init_status_leds();
	
	// Setup UART


	
	
	while (1)
	{
		OpenUART1(UART_EN, UART_TX_ENABLE, 25);
		ConfigIntUART1(UART_RX_INT_DIS | UART_TX_INT_DIS);

		mLED1_Off()
		mLED2_Off()
		mLED2_Off()
		
		//Setup Timeout timer, start disabled
		init_mb_timeout_timer(20000);
		
		// Block until we hear something on the UART
		len = modbusRecvLoop();
		
		CloseUART1();
		if (len != -1)
		{
			result = checkPDU(len, &mb_req_pdu);
			//processPDU(tx_pdu, rx_pdu);
			//formatPDU(tx_pdu, rx_pdu);
			//state = transmitPDU(void);
			WriteUART1('A');
			//	LATFbits.LATF4 = 0;
			//	LATGbits.LATG6 = 0;
			//	LATGbits.LATG8 = 1;
			//while (TMR1 < 20000);
			mLED2_Toggle()
			//TMR1 = 0x0000;
		}
	}
}

inline void __attribute__((always_inline)) txByte(unsigned char byte)
{
	while(BusyUART1());
	WriteUART1(byte);
}

unsigned char rxByte()
{
	unsigned char byte;
	byte = ReadUART1();
	return byte;
}

unsigned char rdyByte()
{
	return(U1STAbits.URXDA);
}


void DebugLED1()
{
	mLED1_On()
	mLED2_Off()
	mLED3_Off()
}

void DebugLED2()
{
	mLED1_Off()
	mLED2_On()
	mLED3_Off()
}

void DebugLED3()
{
	mLED1_Off()
	mLED2_Off()
	mLED3_On()
}
