#define USE_AND_OR //Important for p24 peripheral library

//target selection and h/w specific configs
#include "global.h"
#include "HardwareProfiles.h"
#include "SensorProfiles.h"

//p24 libs
#include <uart.h>
#include <timer.h>

#include "modbus.h"
#include "adc.h"
#include "i2c_engscope.h"
#include "eko_i2c_sensors.h"

//mb crc tables
#include "crc_tables.h"

// MODBUS specific structs and states
slaveStateType 		mb_state; 				// enum type for state
pduType 			mb_resp_pdu; 			// pdu holding struct


volatile unsigned char		mb_data_recvd = 0x00;
volatile unsigned char		mb_pdu_length = 0x00;

// Receive and Transmit Buffers. MAX_DATA_LENGTH is defined in Modbus.h
unsigned char rx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)


/* ISRs */

// ignore message if packet is being received but timeout exceeds threshold
void __attribute__ ((interrupt,no_auto_psv)) _T1Interrupt (void)
{
	T1_Clear_Intr_Status_Bit;
	// defensive programming resets recvd length iff data_recvd = false
	if ((mb_pdu_length > 0) && (mb_data_recvd == 0))
		mb_pdu_length = 0;
}

void __attribute__ ((interrupt,no_auto_psv)) _U1RXInterrupt(void)
{
    // for function codes 0x01 to 0x06, the expected length is 8.
    // we only support functions 0x01 to 0x06.
    static unsigned int expected_pdu_length = 8;
    
	U1RX_Clear_Intr_Status_Bit;
	
	T1CONbits.TON = 0; // disable the timer
	TMR1 = 0x00; // reset count
 	
	while(!DataRdyUART1()); // wait for buffer
    rx_buffer[mb_pdu_length++] = ReadUART1();
    
	if(mb_pdu_length == expected_pdu_length)
		mb_data_recvd=1; // flag that a pdu is recvd
    else
        // re-enable timer and resume if message incomplete
    	T1CONbits.TON = 1;
}



/* MODBUS Message Timeout Timer */

// initialises Timer1 for a timeout count
void init_mb_timeout_timer(unsigned int timer_ticks)
{
	CloseTimer1();
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
	
	#ifdef TARG_EKOBB_R1
		TRISAbits.TRISA2 = 0;
		TRISAbits.TRISA3 = 0;
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
  
	unsigned int i;
	
	// Set up Clock
	OSCCON	=	0x11C0;	 //select INTERNAL RC, Post Scale PPL (fixme)
	
	#ifdef TARG_PIC24F_SK
		// Development Kit needs port mapping
		RPINR18bits.U1RXR = 3;
		RPOR2bits.RP4R = 3;
	#endif
	
	init_status_leds();


	#ifdef SENSORKIT_ATMOS_R1
	mcp9800_init();
	tsl2561_init();
	#endif

	while (1)
	{
     	CloseUART1();
     	ConfigIntUART1(UART_RX_INT_EN | UART_RX_INT_PR2 | UART_TX_INT_DIS);
		OpenUART1(UART_EN, UART_TX_ENABLE, 25);

		// Setup Timeout timer, start disabled
		init_mb_timeout_timer(20000);
		
		// reset byte count and received flag
		mb_pdu_length = 0;
		mb_data_recvd = 0;
		
		// Block until we hear something on the UART
		while (!mb_data_recvd);
		
		// close UART
		CloseUART1();
		
		// if len != 8, then skip processing
		if (mb_pdu_length == 8)
		{
			//result = check_req_pdu(len);
			
			//if (result == MSG_OK) 
			//	result = process_req_pdu();
			
			//CALL ADC SOMEWHERE HEREEEEEEEEEEEEEEEEEEEEEEEEEEE <- Just so you can see me :P
			
			
			//len = format_resp_pdu(result);
			//mLED3_On()
			//OpenUART1(UART_EN, UART_TX_ENABLE, 25);
            //ConfigIntUART1(UART_RX_INT_DIS | UART_TX_INT_DIS);
			
			//init_mb_timeout_timer(6400);
			//start_mb_timeout_timer();
			//while(!mb_req_timeout);
			//close_mb_timeout_timer();
			
			//putsUART1((unsigned int *) tx_buffer);

			//for (i = 0; i < len+1; i++)
			//{
			//txByte(0x00FF & tx_buffer[i]);    /* transfer data word to TX reg */
			//}
			//state = transmitPDU(void);
			
			//WriteUART1('A');
			//	LATFbits.LATF4 = 0;
			//	LATGbits.LATG6 = 0;
			//	LATGbits.LATG8 = 1;
			//while (TMR1 < 20000);
			//mLED3_Off()
			//TMR1 = 0x0000;
			//CloseUART1();
		}
	}
}
