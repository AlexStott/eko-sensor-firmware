/* ********************************************************************* *
 * 				Energy Kiosk Observer : Sensor Firmware					 *
 * 						Version 2 / Aug 2011							 *
 *  																	 *
 * ********************************************************************* *
 * 		AUTHORS:		Charith Amarasinghe <ca508@ic.ac.uk>			 *
 *						Gonzalo de Gisbert	<gdg90@ic.ac.uk>	         *
 *																		 *
 * ********************************************************************* *
 */

#define USE_AND_OR
#define EN_CFG_EEPROM

#include "board/chip.h"
#include "board/p24_fuses.h"
#include "board/p24_board.h"
#include "include/modbus2.h"
#include "include/tmr2delay.h"
#include "include/mb_crc16.h"
#include "include/i2c.h"

/**
 * Cycle Clock Frequency.
 * FCY = FOSC/2 for use in timing routines. 
 */
#define FCY		(4000000)

/**
 * Default device address.
 */
#define DEFAULT_ADDR 0x01

/* Buffer sizes */
#define RX_BUF_MAX		25 /*!< Receive Buffer Size. limited to 16 bytes data + 9 bytes mb stuff. */
#define TX_BUF_MAX		70 /*!< Transmit Buffer Size. limited to 32 words data + 5 bytes mb stuff. */
#define DATA_BUF_SIZE	64 /*!< Data Buffer Size. eg: capable of 16 samples per input accross 4 inputs. */

/**
 * Modbus device address.
 * Modbus device adddress as loaded from EEPROM.
 */
unsigned char mb_addr;

/* transmit and receive buffers */
unsigned char rxbuf[RX_BUF_MAX]; /*!< UART receive buffer. */
unsigned char txbuf[TX_BUF_MAX]; /*!< UART transmit buffer. */

/* bytes in rx buffer */
unsigned char rx_len; /*!< Received byte count. Bytes read consecutively from UART before threshold or timeout. */


/* main data buffer */
unsigned int  databuf[DATA_BUF_SIZE]; /*!< Analogue data buffer. */

/**
 * Configuration data buffer.
 */
unsigned char confbuf[DATA_BUF_SIZE]; /*!< Configuration / Status buffer. */

/**
 * Configuration EEPROM dirty marker.
 * Set to 1 if the EEPROM needs to be updated with data in the conf buffer
 */
unsigned char cfg_eeprom_dirty;


void __attribute__((interrupt, auto_psv)) _DefaultInterrupt(void)
{
  while (1)
  {
      Nop();
      Nop();
      Nop();
  }
}

void __attribute__((interrupt, auto_psv)) _OscillatorFail(void)
{
  while (1)
  {
      Nop();
      Nop();
      Nop();
  }
}
void __attribute__((interrupt, auto_psv)) _AddressError(void)
{
  while (1)
  {
      Nop();
      Nop();
      Nop();
  }
}
void __attribute__((interrupt, auto_psv)) _StackError(void)
{
  while (1)
  {
      Nop();
      Nop();
      Nop();
  }
}
void __attribute__((interrupt, auto_psv)) _MathError(void)
{
  while (1)
  {
      Nop();
      Nop();
      Nop();
  }
}

/**
 * Initialise routine for UART1.
 * @param BRG the baud rate clock count value as calculated from FCY and desired serial baudrate.
 */
void init_uart1(int BRG)
{
	U1BRG = BRG;
	U1MODE = 0x8000; /* Enable UART */
	U1STA = 0x0400; /* Enable transmit */
}

/**
 * Turn error led on.
 */
void error_led_on( void )
{
	mLED2_On()
}

/**
 * Block until UART receives modbus message.
 */
char get_msg( void )
{
	unsigned char idx = 0;
	unsigned char thresh = 8;
	unsigned char temp;
	
	// wait until data available bit is set
	while ( !U1STAbits.URXDA  ) {}
	
	// Use TMR2 for timeout of around 10ms
	TMR2 = 0;
	T2CON = 0x8030; // 1:256 prescale, enable tmr2
	
 	while ( (idx < thresh) && (TMR2 < 150) && (idx < RX_BUF_MAX) )
	{
		// empty buffer while data available
		while(U1STAbits.URXDA)
		{
			rxbuf[idx++] = (unsigned char)(0x00FF & U1RXREG);
		}
		// check for function code 16
		if ((idx == 7) && (rxbuf[2] == 0x10))
		{
			// set pdu length for extra bytes
			thresh = 9 + rxbuf[6];
		}
		TMR2 = 0;
	}
	
	// disable timer
	T2CON = 0x0000;

	// flush buffer of extra data
	while( U1STAbits.URXDA )
	{
		temp = (unsigned char)(0x00FF & U1RXREG); // dump extra characters
	}

	return idx;
}

void init_ports( void )
{
	//TRIS Settings for UART
	#ifdef TARGET_EKOBB_R3
		TRISBbits.TRISB2 = 1; // In
		TRISBbits.TRISB7 = 0; // Out
		TRISBbits.TRISB8 = 0;
	#endif
	
	// TRIS Settings for I2C
	#ifdef TARGET_EKOBB_R3
		TRISBbits.TRISB5 = 1;
		TRISBbits.TRISB6 = 1;
	#endif
	
	// TRIS Settings for Interrupt
	#ifdef TARGET_EKOBB_R3
		TRISAbits.TRISA2 = 0;
	#endif

	// Setup Analogue Ports:
	//     Set TRIS bits to 1 (Input)
	//	   Set AD1PCFG bits to 0 (ADC Input)
	#ifdef TARGET_EKOdBB_R3
		TRISAbits.TRISA0 = 1; // AN0
		TRISAbits.TRISA1 = 1; // AN1
		TRISBbits.TRISB0 = 1; // AN2
		TRISBbits.TRISB1 = 1; // AN3
		TRISBbits.TRISB3 = 1; // AN5
		AD1PCFG = 0xFFE0; // AN0-AN3 and AN5 set as analogue
	#endif
}

void load_cfg_from_eeprom( void )
{
	#ifdef EN_CFG_EEPROM
	unsigned int cfg_crc;
	
	InitI2C(); /* Initialise I2C at 100kHz @ 8Mhz FCY */
	LDSequentialReadI2C(EE_ADDR_CFG, 0x00, confbuf, 16);
	cfg_crc = calculate_crc16(confbuf, 14);
	if (cfg_crc == ((((unsigned int)confbuf[15] << 8) & 0xFF00) + (unsigned int)confbuf[14]))
	{
		// Configure
	}
	else
	{
		error_led_on();
	#endif
		// Configure defaults
	#ifdef EN_CFG_EEPROM
	}
	CloseI2C();
	#endif
	
	cfg_eeprom_dirty = 0;
}

void save_cfg_to_eeprom( void )
{
	// if eeprom needs to be saved, save it.
	unsigned int cfg_crc;
	
	#ifdef EN_CFG_EEPROM
		cfg_crc = calculate_crc16(confbuf, 14);
		confbuf[15] = (unsigned char)((cfg_crc >> 8) & 0x00FF);
		confbuf[14] = (unsigned char)(cfg_crc & 0x00FF);
		CloseI2C();
		InitI2C();
		LDPageWriteI2C(EE_ADDR_CFG, 0x00, confbuf);
		// blink leds
		mLED1 = 0;
		mLED2 = 0;
		for(cfg_crc = 0; cfg_crc < 20; cfg_crc++)
		{
		delay_ms(100);
		mLED1 = !mLED1;
		mLED2 = !mLED2;
		}
		mLED1 = 0;
		mLED2 = 0;
		CloseI2C();
		
		
	#endif
	
	cfg_eeprom_dirty = 0;
}

int main (void)
{

	unsigned char temp = 0;
	unsigned char i;
	
	AD1PCFG = 0xFFFF; // All pins digital to begin

	/* UART1_RTS aliases a digital output pin (PORTB5?),
	   For the SP483E to transmit, set RTS pin high
	   For receive set RTS low.
       !!Tx FIFO is 4 bytes. After we write the last
		4 bytes to the tx reg, keep RTS high for approx 10ms
		for last few data bytes to transmit */  

	UART1_RTS = 0; // Receiver Enabled
	#ifdef TARGET_EKOBB_R3
	BUS_INTERRUPT = 1; // set interrupt line to O/D
	#endif
	init_ports();
	mInitLED();
	load_cfg_from_eeprom();

	for (i = 0; i < 20; i++)
	{	
		confbuf[i] = i;
		databuf[i] = (unsigned int)(i*100);
	}

	while(1)
	{	
		// init uart and interrupts
		rx_len = 0;
		init_uart1(25);
		mLED1 = 0;
		mLED2 = 0;
  		//if (U1STAbits.URXDA)
		rx_len = get_msg();
		// message recvd. in rxbuf.
		temp = validate_pdu(DEFAULT_ADDR, rx_len, rxbuf);
		
		
		if (temp == 1)
		{
			mLED2 = 0; // reset error led
			// message is known good!
			mLED1 = 1; // frame led on
			temp = process_pdu(rxbuf, txbuf, databuf, confbuf);
		
	
			/* Transmit Response */
			UART1_RTS = 1; // TXEN
	
			delay_ms(2);
	
			for(i=0; i<temp; i++)
			{
				while (U1STAbits.UTXBF);
				U1TXREG = (unsigned int)(txbuf[i]) & 0x00FF;
			}

			mLED1 = 0; // frame led off
		}
		delay_ms(15);

		UART1_RTS = 0;
		
		
		if ((cfg_eeprom_dirty == 1) && (confbuf[16] == 0xA0) && (confbuf[17] == 0xEE))
		{
			save_cfg_to_eeprom();
		}
		Nop();
		Nop();
	}
}

