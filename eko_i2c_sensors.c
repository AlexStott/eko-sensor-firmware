
#define FCY 2000000UL
#include <libpic30.h>
#include "i2c_engscope.h"

#define I2C_MCP9800_ADDR 0x90
#define I2C_MCP9800_TA 0x00

#define I2C_TSL2561_ADDR 0x72
#define I2C_TSL2561_CONTROL 0x80
#define I2C_TSL2561_CONTROL_PWRON 0x03
#define I2C_TSL2561_DATA0LOW 0x8C
#define I2C_TSL2561_DATA0HIGH 0x8D
#define I2C_TSL2561_DATA1LOW 0x8E
#define I2C_TSL2561_DATA1HIGH 0x8F

extern unsigned int CalculateLux(unsigned int iGain, unsigned int tInt, unsigned int ch0, unsigned int ch1, int iType);

void mcp9800_init()
{
	// set maximum resolution
	I2Cwrite(I2C_MCP9800_ADDR, 0x01, 0x30);
}
int mcp9800_get_temp()
{
	unsigned int temp;
	unsigned char sign;
	temp = I2Cread(I2C_MCP9800_ADDR, I2C_MCP9800_TA) << 8;
	temp = temp + (0xFF & I2Cread(I2C_MCP9800_ADDR, I2C_MCP9800_TA));
	sign = (temp & 0x8000) >> 15; // get MSB
	if (sign)
	{
		return ((0x7FFF & temp) >> 4);
	} else
	{
		return -((0x7FFF & temp) >> 4);
	}
}

void tsl2561_init()
{
	// Write the register address for writing to the command register
	i2c_start();
	send_i2c_byte(I2C_TSL2561_ADDR);
	send_i2c_byte(I2C_TSL2561_CONTROL);
	i2c_restart();
	send_i2c_byte(I2C_TSL2561_ADDR);
	send_i2c_byte(I2C_TSL2561_CONTROL_PWRON);
	reset_i2c_bus();
}

unsigned char tsl2561_get_reg(unsigned char command_reg_value)
{
	unsigned char data;
	i2c_start();
	send_i2c_byte(I2C_TSL2561_ADDR); // Write to optical sensor
	send_i2c_byte(command_reg_value);
	i2c_restart();
	send_i2c_byte(I2C_TSL2561_ADDR | 0x01);
	data = i2c_read();
	reset_i2c_bus();
	return data;
}

unsigned int tsl2561_get_lux()
{
	unsigned char data0l, data0h, data1l, data1h;
	unsigned int data1, data0;
	data0l = tsl2561_get_reg(I2C_TSL2561_DATA0LOW);
	data0h = tsl2561_get_reg(I2C_TSL2561_DATA0HIGH);
	data1l = tsl2561_get_reg(I2C_TSL2561_DATA1LOW);
	data1h = tsl2561_get_reg(I2C_TSL2561_DATA1HIGH);
	
	data0 = ((data0h << 8) & 0xFF00) + (data0l & 0x00FF);
	data1 = ((data1h << 8) & 0xFF00) + (data1l & 0x00FF);
	return CalculateLux(0, 2, data0, data1, 0);
}

