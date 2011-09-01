#include "i2c_engscope.h"
#define FCY 2000000UL
#include <libpic30.h>
//loop nops for delay
void DelayuSec(unsigned int N)
{
   unsigned int j;
   while(N > 0)
   {
      for(j=0;j < 10; j++);
         N--;
   }
}

//function initiates I2C1 module to baud rate BRG
void i2c_init(int BRG)
{
   int temp;

   // I2CBRG = 19 for 8Mhz OSCI with 1:2 PS and 100kHz I2C clock
   I2C1BRG = BRG;
   I2C1CONbits.I2CEN = 0;	// Disable I2C Mode
   I2C1CONbits.DISSLW = 1;	// Disable slew rate control
   IFS1bits.MI2C1IF = 0;	 // Clear Interrupt
   I2C1CONbits.I2CEN = 1;	// Enable I2C Mode
   temp = I2CRCV;	 // read buffer to clear buffer full
   reset_i2c_bus();	 // set bus to idle
}


void i2c_restart(void)
{
   int x = 0;
   I2C1CONbits.RSEN = 1;	//Initiate Start condition
   Nop();

   //the hardware will automatically clear Start Bit
   //wait for automatic clear before proceding
   while (I2C1CONbits.RSEN)
   {
      __delay_us(1);
      x++;
      if (x > 20)
      break;
   }
   __delay_us(2);
}

//function iniates a start condition on bus
void i2c_start(void)
{
   int x = 0;
   I2C1CONbits.ACKDT = 0;	//Reset any previous Ack
   __delay_us(10);
   I2C1CONbits.SEN = 1;	//Initiate Start condition
   Nop();

   //the hardware will automatically clear Start Bit
   //wait for automatic clear before proceding
   while (I2C1CONbits.SEN)
   {
      __delay_us(1);
      x++;
      if (x > 20)
      break;
   }
   __delay_us(2);
}
//Resets the I2C bus to Idle
void reset_i2c_bus(void)
{
   int x = 0;

   //initiate stop bit
   I2C1CONbits.PEN = 1;

   //wait for hardware clear of stop bit
   while (I2C1CONbits.PEN)
   {
      __delay_us(1);
      x ++;
      if (x > 20) break;
   }
   I2C1CONbits.RCEN = 0;
   IFS1bits.MI2C1IF = 0; // Clear Interrupt
   I2C1STATbits.IWCOL = 0;
   I2C1STATbits.BCL = 0;
   __delay_us(10);
}

//basic I2C byte send
char send_i2c_byte(int data)
{
   int i;

   while (I2C1STATbits.TBF) { }
   IFS1bits.MI2C1IF = 0; // Clear Interrupt
   I2CTRN = data; // load the outgoing data byte

   // wait for transmission
   for (i=0; i<500; i++)
   {
      if (!I2C1STATbits.TRSTAT) break;
      __delay_us(1);

      }
      if (i == 500) {
      return(1);
   }

   // Check for NO_ACK from slave, abort if not found
   if (I2C1STATbits.ACKSTAT == 1)
   {
      reset_i2c_bus();
      return(1);
   }

   __delay_us(2);
   return(0);
}


//function reads data, returns the read data, no ack
char i2c_read(void)
{
   int i = 0;
   char data = 0;

   //set I2C module to receive
   I2C1CONbits.RCEN = 1;

   //if no response, break
   while (!I2C1STATbits.RBF)
   {
      i ++;
      if (i > 2000) break;
   }

   //get data from I2CRCV register
   data = I2CRCV;

   //return data
   return data;
}
//function reads data, returns the read data, with ack
char i2c_read_ack(void)	//does not reset bus!!!
{
   int i = 0;
   char data = 0;

   //set I2C module to receive
   I2C1CONbits.RCEN = 1;

   //if no response, break
   while (!I2C1STATbits.RBF)
   {
      i++;
      if (i > 2000) break;
   }

   //get data from I2CRCV register
   data = I2CRCV;

   //set ACK to high
   I2C1CONbits.ACKEN = 1;

   //wait before exiting
   __delay_us(10);

   //return data
   return data;
}

unsigned char I2Cpoll(char addr)
{
   unsigned char temp = 0;

   i2c_start();
   temp = send_i2c_byte(addr);
   reset_i2c_bus();

   return temp;
}

// write to an address
void I2Cwrite(char addr, char subaddr, char value)
{
	char tmp;
	i2c_start();
	tmp = send_i2c_byte(addr);
	tmp = send_i2c_byte(subaddr);
	tmp = send_i2c_byte(value);
	reset_i2c_bus();
}

//read from an address
char I2Cread(char addr, char subaddr)
{
	char temp;
	i2c_start();
	send_i2c_byte(addr);
	send_i2c_byte(subaddr);
	__delay_us(10);
	i2c_restart();
	send_i2c_byte(addr | 0x01);
	temp = i2c_read();
	reset_i2c_bus();
	return temp;
}

/* Writes a word to the EEPROM */
void I2Cwritedouble(char addr, char subaddr, unsigned int value)
{
	char valueHigh = (0xFF00 & value) >> 8;
	char valueLow = (char)(0x00FF & value);
	
	i2c_start();
	send_i2c_byte(addr);
	send_i2c_byte(subaddr);
	send_i2c_byte(valueLow);
	send_i2c_byte(valueHigh);
	reset_i2c_bus();
}

/* Reads a word from the eeprom */
unsigned int I2Creaddouble (char addr, char subaddr)
{
	char valueHigh;
	char valueLow;
	
	i2c_start();
	send_i2c_byte(addr);
	send_i2c_byte(subaddr);
	DelayuSec(50);
	
	i2c_restart();
	send_i2c_byte(addr | 0x01);
	valueLow = i2c_read_ack();
	valueHigh = i2c_read();
	reset_i2c_bus();
	
	return ((valueHigh << 8 ) & 0xFF00) + valueLow;
}
