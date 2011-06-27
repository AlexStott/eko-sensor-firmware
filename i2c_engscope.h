#ifndef I2C_E_H
#define I2C_E_H
#include "p24F16KA102.h"


#define I2CRCV I2C1RCV
#define I2CTRN I2C1TRN


void DelayuSec(unsigned int N);

void i2c_init(int BRG);

void i2c_start(void);

void reset_i2c_bus(void);

char send_i2c_byte(int data);

char i2c_read(void);

char i2c_read_ack(void);


unsigned char I2Cpoll(char addr);

void I2Cwritedouble(char addr, char subaddr, unsigned int value);

unsigned int I2Creaddouble (char addr, char subaddr);

#endif
