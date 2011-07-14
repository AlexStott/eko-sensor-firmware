#define USE_AND_OR

#include "global.h"
#include "HardwareProfiles.h"
#include "SensorProfiles.h"
#include "eko_i2c_sensors.h"
#include "i2c_engscope.h"
#include "modbus.h"
#include "adc.h"
#include <uart.h>
#include <timer.h>
//#include "eko_i2c_sensors.h"

#include "crc_tables.h"


// Configs defined in HardwareProfiles.h

// MODBUS specific structs and states
slaveStateType 		mb_state; 				// enum type for state
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
	
	#ifdef TARG_EKOBB_R1
		TRISAbits.TRISA2 = 0;
		TRISAbits.TRISA3 = 0;
	#endif
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
	
	#ifdef TARG_EKOBB_R1
		// Set UART_RTS pin as digital output
		TRISBbits.TRISB8 = 0;

		// Set UART_RTS pin high, schematic defect in R1 and R2 means inverted from H/W sense
		LATBbits.LATB8 = 1;
		
		// Set UART_TX pin as digital output
		TRISBbits.TRISB7 = 0;
		
		// Set UART_RX pin as digital input
		TRISBbits.TRISB2 = 1;
	#endif
	
	init_status_leds();

	AD1PCFG = 0xFFFF;
	#ifdef SENSORKIT_ATMOS_R1
		i2c_init(89);	
		mcp9800_init();
		tsl2561_init();
	#endif

	// Initialise UART
	U1BRG = 25; // thats 9600bps with 8MHz FRC no post-scale/PLL
	U1MODE = 0x8000; // Enable UART
	U1STA = 0x0400; // Enable transmit and enable interrupt
	//IFS0bits.U1RXIF = 0;
	while (1)
	{
		//IFS0bits.U1RXIF = 0;
		UART_RTS = 1; // Set SP483 to rx mode
		
		//Setup Timeout timer, start disabled
		init_mb_timeout_timer(20000);
		
		// Block until we hear something on the UART
		len = modbusRecvLoop();
		
		// if len = MODBUS_NOT_ADDR, then skip processing
		//if (len != MODBUS_NOT_ADDR)
		if (rx_buffer[0] == MODBUS_ID)
		{
			result = check_req_pdu(len);
			
			if (result == MSG_OK) 
				result = process_req_pdu();
			
			len = format_resp_pdu(result);
			//frame LED on
			
			//OpenUART1(UART_EN, UART_TX_ENABLE, 25);
			//ConfigIntUART1(UART_RX_INT_DIS | UART_TX_INT_DIS);
			
			// Set SP483 to tx mode
			UART_RTS = 0;
//			
			init_mb_timeout_timer(6400);
			start_mb_timeout_timer();
			while(!mb_req_timeout);
			close_mb_timeout_timer();
			mLED2_On()
			//putsUART1((unsigned int *) tx_buffer);

			for (i = 0; i < len+1; i++)
			{
			txByte(0x00FF & tx_buffer[i]);    /* transfer data word to TX reg */
			}
			//state = transmitPDU(void);
			
			//WriteUART1('A');
			//LATFbits.LATF4 = 0;
			//	LATGbits.LATG6 = 0;
			//	LATGbits.LATG8 = 1;
			//while (TMR1 < 20000);
			// frame LED off
		
			//TMR1 = 0x0000;
		}
		else
		{
				init_mb_timeout_timer(2000);
				start_mb_timeout_timer();
				status_leds_frame_off();
				while(!mb_req_timeout);
		}
		//LATAbits.LATA2 = 1;
		//mLED2_Off()
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


void status_leds_off()
{
       mLED1_Off()
       mLED2_Off()
       mLED3_Off()
}

void status_leds_frame_on()
{
        mLED2_On()
}

void status_leds_frame_off()
{
        mLED2_Off()
}