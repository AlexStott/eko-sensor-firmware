#define USE_AND_OR

#include "global.h"
#include "HardwareProfiles.h"
#include "SensorProfiles.h"

#include "modbus.h"
#include "adc.h"
#include <uart.h>
#include <timer.h>

#include "crc_tables.h"


extern	unsigned int	data1[10];
	
// Configs defined in HardwareProfiles.h

// MODBUS specific structs and states
slaveStateType 	mb_state; 				// enum type for state
pduType 			mb_req_pdu; 			// pdu holding struct
pduType 			mb_resp_pdu;


volatile unsigned char		mb_req_timeout = 0x00;


// Receive and Transmit Buffers. MAX_DATA_LENGTH is defined in Modbus.h
unsigned char rx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)
unsigned char tx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)

/* ISRs */

//  Timeout
void __attribute__ ((interrupt,no_auto_psv)) _T1Interrupt (void)
{
   T1_Clear_Intr_Status_Bit;
//	if (mb_state == SLAVE_RECEIVING)
		mb_req_timeout = 1;
	
}


/* MODBUS Message Timeout Timer */

// initialises Timer1 for a timeout count
void init_mb_timeout_timer(unsigned int timer_ticks)
{
	CloseTimer1();
	mb_req_timeout = 0;
	OpenTimer1(T1_OFF | T1_PS_1_256, timer_ticks);
	ConfigIntTimer1(T1_INT_ON|T1_INT_PRIOR_1);
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

/* MODBUS CRC calculation PIC24F S/W */

unsigned int calculate_crc16(unsigned char *puchMsg, unsigned int usDataLen)
{
	unsigned char uchCRCHi = 0xFF ; /* high byte of CRC initialized  */ 
	unsigned char uchCRCLo = 0xFF ; /* low byte of CRC initialized  */ 
	unsigned int uIndex ; /* will index into CRC lookup table  */ 
	while (usDataLen--) /* pass through message buffer  */ 
	{ 
		uIndex = uchCRCLo ^ *puchMsg++ ;  /* calculate the CRC   */ 
		uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex] ; 
		uchCRCHi = auchCRCLo[uIndex] ; 
	} 
	return (uchCRCHi << 8 | uchCRCLo);
}

/* Status LEDs */

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
	unsigned int i;
	// Set up Clock
	OSCCON	=	0x11C0;	 //select INTERNAL RC, Post Scale PPL (fixme)
	
	#ifdef TARG_PIC24F_SK
		// Development Kit needs port mapping
		RPINR18bits.U1RXR = 3;
		RPOR2bits.RP4R = 3;
	#endif
	init_status_leds();
	
	data1[0] = 0x1234;
	data1[1] = 0x5678;
	data1[2] = 0x9ABC;
	data1[3] = 0xDEF0;
	data1[4] = 0xAAAA;
	data1[5] = 0xBBBB;
	data1[6] = 0xCCCC;
	data1[7] = 0xDDDD;
	
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
		// if len = MODBUS_NOT_ADDR, then skip processing
		if (len != MODBUS_NOT_ADDR)
		{
			result = check_req_pdu(len);
			
			if (result == MSG_OK) 
				result = process_req_pdu();
			
			//CALL ADC SOMEWHERE HEREEEEEEEEEEEEEEEEEEEEEEEEEEE <- Just so you can see me :P
			//ADCmain();
			len = format_resp_pdu(result);
			mLED3_On()
			OpenUART1(UART_EN, UART_TX_ENABLE, 25);
			ConfigIntUART1(UART_RX_INT_DIS | UART_TX_INT_DIS);
			
			init_mb_timeout_timer(6400);
			start_mb_timeout_timer();
			while(!mb_req_timeout);
			close_mb_timeout_timer();
			
			//putsUART1((unsigned int *) tx_buffer);

			for (i = 0; i < len+1; i++)
			{
			txByte(0x00FF & tx_buffer[i]);    /* transfer data word to TX reg */
			}
			//state = transmitPDU(void);
			
			//WriteUART1('A');
			//	LATFbits.LATF4 = 0;
			//	LATGbits.LATG6 = 0;
			//	LATGbits.LATG8 = 1;
			//while (TMR1 < 20000);
			mLED3_Off()
			//TMR1 = 0x0000;
			CloseUART1();
		}
	}
}

void txByte(unsigned char byte)
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
