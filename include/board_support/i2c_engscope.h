#ifndef I2C_E_H
#define I2C_E_H
#include "p24Fxxxx.h"


#define I2CRCV I2C1RCV
#define I2CTRN I2C1TRN


void DelayuSec(unsigned int N);

void i2c_init(int BRG);

void i2c_start(void);

void i2c_restart(void);

void reset_i2c_bus(void);

char send_i2c_byte(int data);

char i2c_read(void);

char i2c_read_ack(void);


unsigned char I2Cpoll(char addr);

void I2Cwritedouble(char addr, char subaddr, unsigned int value);

unsigned int I2Creaddouble (char addr, char subaddr);

char I2Cread(char addr, char subaddr);

void I2Cwrite(char addr, char subaddr, char value);

#endif
