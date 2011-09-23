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

#include "board/chip.h"
#include "board/p24_fuses.h"
#include "board/p24_board.h"
#include "include/modbus2.h"
#include "include/tmr2delay.h"
#include "include/mb_crc16.h"
#include "include/i2c.h"
#include "include/p24_adc.h"
#include "include/eko_i2c_sensors.h"

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
#define DATA_BUF_SIZE	256 /*!< Data Buffer Size. eg: capable of 16 samples per input accross 4 inputs. */
#define CONF_BUF_SIZE	64
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
unsigned char confbuf[CONF_BUF_SIZE]; /*!< Configuration / Status buffer. */

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
	
 	while ( (idx < thresh) && (TMR2 < 20) && (idx < RX_BUF_MAX) )
	{
		// empty buffer while data available
		while(U1STAbits.URXDA)
		{
			rxbuf[idx++] = (unsigned char)(0x00FF & U1RXREG);
			TMR2 = 0;
		}
		// check for function code 16
		if ((idx == 7) && (rxbuf[2] == 0x10))
		{
			// set pdu length for extra bytes
			thresh = 9 + rxbuf[6];
		}
		
	}
	
	// disable timer
	T2CON = 0x0000;

	// flush buffer of extra data
	while( U1STAbits.URXDA )
	{
		temp = (unsigned char)(0x00FF & U1RXREG); // dump extra characters
	}
	U1STA = 0x0400;
	return idx;
}

void init_ports( void )
{
	//TRIS Settings for UART
	#if defined(TARGET_EKOBB_R3) || defined(TARGET_EKOBB_R2)
		TRISBbits.TRISB2 = 1; // In
		TRISBbits.TRISB7 = 0; // Out
		TRISBbits.TRISB8 = 0;
	#endif
	// TRIS Settings for I2C
	#if defined(TARGET_EKOBB_R3) || defined(TARGET_EKOBB_R2)
		TRISBbits.TRISB5 = 1;
		TRISBbits.TRISB6 = 1;
	#endif
	
	// TRIS Settings for Interrupt
	#if defined(TARGET_EKOBB_R3)
		TRISAbits.TRISA2 = 0;
	#endif

	// Setup Analogue Ports:
	//     Set TRIS bits to 1 (Input)
	//	   Set AD1PCFG bits to 0 (ADC Input)
	#if defined(TARGET_EKOBB_R3)
		TRISAbits.TRISA0 = 1; // AN0
		TRISAbits.TRISA1 = 1; // AN1
		TRISBbits.TRISB0 = 1; // AN2
		TRISBbits.TRISB1 = 1; // AN3
		TRISBbits.TRISB3 = 1; // AN5
		AD1PCFG = 0xFFD0; // AN0-AN3 and AN5 set as analogue
	#endif

	#if defined(TARGET_EKOBB_R2)
		AD1PCFG = 0xFFFF;
	#endif

	// Enable weak internal pull up on CN11	
	#if defined(TARGET_EKOBB_R3) || defined(TARGET_EKOBB_R2)
		TRISBbits.TRISB15 = 1;
		CNPU1bits.CN11PUE = 1; // enable pullup
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
		// save address incase someone erases it
		mb_addr = confbuf[CFG_EE_MBADDR];
		// set control registers to 0
		confbuf[CFG_ADC_CONTROL] = 0;
		confbuf[CFG_I2C_CONTROL] = 0;
	}
	else
	{
		error_led_on();
	#endif
		// Configure defaults
		mb_addr = DEFAULT_ADDR;
		confbuf[CFG_EE_MBCLS] = 0x00;
		confbuf[CFG_EE_ADC_ISEL] = 0x2F;  // enable all AN pins
		confbuf[CFG_EE_ADC_REPT] = 32; // 32 samples
		confbuf[CFG_EE_ADC_WAIT] = 10; // 10us wait
		confbuf[CFG_EE_ADC_SAMP] = 2;  // 2ms wait between samples
		confbuf[CFG_EE_I2C_CHIP] = 0;  // no i2c devices registered
		confbuf[CFG_ADC_CONTROL] = 0;
		confbuf[CFG_I2C_CONTROL] = 0;
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
		confbuf[CFG_EE_CRC_HI] = (unsigned char)((cfg_crc >> 8) & 0x00FF);
		confbuf[CFG_EE_CRC_LO] = (unsigned char)(cfg_crc & 0x00FF);
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
	
	load_cfg_from_eeprom();
	cfg_eeprom_dirty = 0;
}

void reset_configuration( void )
{
	int i = 0;
	int cfg_crc;
	for(i=0; i<16; i++)
	{
		confbuf[i] = 0;
	}
	CloseI2C();
	InitI2C();
	LDPageWriteI2C(EE_ADDR_CFG, 0x00, confbuf);

	mLED1 = 0;
	mLED2 = 0;
	for(cfg_crc = 0; cfg_crc < 20; cfg_crc++)
	{
	delay_ms(75);
	mLED1 = !mLED1;
	}

	CloseI2C();
	cfg_eeprom_dirty = 0;

//	load_cfg_from_eeprom();
}

int main (void)
{

	unsigned int temp = 0;
	unsigned int i;
	
	AD1PCFG = 0xFFFF; // All pins digital to begin

	/* UART1_RTS aliases a digital output pin (PORTB5?),
	   For the SP483E to transmit, set RTS pin high
	   For receive set RTS low.
       !!Tx FIFO is 4 bytes. After we write the last
		4 bytes to the tx reg, keep RTS high for approx 10ms
		for last few data bytes to transmit */  

	UART1_RTS = BUSRX; // Receiver Enabled
	#ifdef TARGET_EKOBB_R3
	BUS_INTERRUPT = 1; // set interrupt line to O/D
	#endif
	init_ports();
	mInitLED();
	mLED1 = 0;
	mLED2 = 0;
	
	// clear buffers
	for (i = 0; i <= 255; i++)
		{
			databuf[i] = 0;
		}
	for (i = 0; i < 64; i++)
		{
			confbuf[i] = 0;
		}
	
	if (RESET_EEPROM == 0)
	{
		reset_configuration();
	}

	load_cfg_from_eeprom();

	// if present, init the mcp9800
	if ((confbuf[CFG_EE_I2C_CHIP] & 0x80) != 0)
	{
		CloseI2C();
		InitI2C();
		mcp9800_init();
		CloseI2C();
	}

	if ((confbuf[CFG_EE_I2C_CHIP] & 0x40) != 0)
	{
		CloseI2C();
		InitI2C();
		tsl2561_init();
		CloseI2C();
	}


	while(1)
	{	
		// init uart and interrupts
		rx_len = 0;
		init_uart1(25);
	
		confbuf[SYS_FW_VERSION] = 0x03;
		#ifdef TARG_EKOBB_R2
			confbuf[SYS_HW_VERSION = 0x02;
		#endif

		#ifdef TARG_EKOBB_R3
			confbuf[SYS_HW_VERSION = 0x03;
		#endif

  		// failsafe, check address validity and reset
		if (mb_addr == 0)
		{
			mLED2 = 1;
			mb_addr = 0x01;
		}
		
	
		// block until pdu received
		rx_len = get_msg();

		// reset LEDs
		mLED1 = 0;

		// message recvd. in rxbuf.
		temp = validate_pdu(mb_addr, rx_len);



	
		if (temp == 1)
		{
			mLED2 = 0; // reset error led
		
			// message is known good!
			
			mLED1 = 1; // frame led on
			temp = process_pdu();
		
			/* Transmit Response */
			UART1_RTS = BUSTX; // TXEN
	
			delay_ms(2);
	
			for(i=0; i<temp; i++)
			{
				while (U1STAbits.UTXBF);
				U1TXREG = (unsigned int)(txbuf[i]) & 0x00FF;
			}

			mLED1 = 0; // frame led off
		}
		delay_ms(15); // wait for txbuffer to empty. 4*11 bits @ 9600bps.

		UART1_RTS = BUSRX;
		
		if (confbuf[CFG_ADC_CONTROL] == 0x80)
		{
			ProcessADCEvents( confbuf[CFG_EE_ADC_ISEL], (unsigned int)confbuf[CFG_EE_ADC_WAIT],
						confbuf[CFG_EE_ADC_REPT], (unsigned int)confbuf[CFG_EE_ADC_SAMP]);
			confbuf[CFG_ADC_CONTROL] = 0x00;
		}
		
		if ((((confbuf[CFG_I2C_CONTROL] & 0x80) != 0) ||
			 ((confbuf[CFG_I2C_CONTROL] & 0x20) != 0)) && 
			  ((confbuf[CFG_EE_I2C_CHIP] & 0x80) != 0))
		{
			mLED1 = 1;
			CloseI2C();
			InitI2C();
			temp = mcp9800_get_temp();
			confbuf[DAT_I2C_TEMP_HI] = (unsigned char)(temp >> 8);
			confbuf[DAT_I2C_TEMP_LO] = (unsigned char)(temp & 0x00FF);
			CloseI2C();
			mLED1 = 0;
			confbuf[CFG_I2C_CONTROL] &= 0x7F;
		}
		
		// Enable only if bit 7 is set [I2C Light Single]
		// or if bit 5 is set [I2C Light continuous]
		if ((((confbuf[CFG_I2C_CONTROL] & 0x40) != 0) ||
			 ((confbuf[CFG_I2C_CONTROL] & 0x10) != 0)) && 
             ((confbuf[CFG_EE_I2C_CHIP] & 0x40) != 0))
		{
			mLED1 = 1;
			CloseI2C();
			InitI2C();
			temp = tsl2561_get_lux();
			confbuf[DAT_I2C_LIGHT_HI] = (unsigned char)(temp >> 8);
			confbuf[DAT_I2C_LIGHT_LO] = (unsigned char)(temp & 0x00FF);
			CloseI2C();
			mLED1 = 0;
			confbuf[CFG_I2C_CONTROL] &= 0xBF;
		}	

		if ((cfg_eeprom_dirty == 1) && (confbuf[CFG_EE_LOCK_HI] == 0xA0) && (confbuf[CFG_EE_LOCK_LO] == 0xEE))
		{
			// write to eeprom only if 0x8008 is set to AOEE
			save_cfg_to_eeprom();
			// reset register contents
			confbuf[CFG_EE_LOCK_LO] = 0x00;
			confbuf[CFG_EE_LOCK_HI] = 0x00;
		}
		
		if((confbuf[CFG_EE_LOCK_HI] == 0xF0) && (confbuf[CFG_EE_LOCK_LO] == 0x0D))
		{
			// fill databuffers with random data for testing
			mLED1 = 1;
			for (i = 0; i <= 255; i++)
			{
				databuf[i] = i*100;
			}
			for (i = 17; i < 64; i++)
			{
				confbuf[i] = i*3;
			}
		}
		else if ((confbuf[CFG_EE_LOCK_HI] == 0xDE) && (confbuf[CFG_EE_LOCK_LO] == 0xAD))
		{
			// fill databuffers with random data for testing
			mLED1 = 1;
			mLED2 = 1;
			for (i = 0; i <= 255; i++)
			{
				databuf[i] = 0;
			}
			for (i = 17; i < 64; i++)
			{
				confbuf[i] = 0;
			}
		}
	
		Nop();
		Nop();
	}
}

