/*
 * i2c.h
 *
 * Created: 04.08.2022 20.06.13
 *  Author: aleks
 */ 


#ifndef I2C_H_
#define I2C_H_

#include <stdbool.h>

#define PIN_USI_SCL		PB2
#define PIN_USI_SDA		PB0
#define PIN_USI_CL		PINB

void i2c_init(void);

uint8_t i2c_read(void);

uint8_t i2c_readLast(void);

bool i2c_write(uint8_t data);

bool i2c_start(uint8_t address, uint16_t readcount);

bool i2c_restart(uint8_t address, uint16_t readcount) ;

void i2c_stop(void);


/*

Writing to an I2C device
Writing to an I2C device is straightforward: for example, to write one byte:

TinyI2C.start(Address, 0);
TinyI2C.write(byte);
TinyI2C.stop();

Reading from an I2C device
The TinyI2C routines allow you to identify the last byte read from an I2C device in either of two ways:

You can specify the total number of bytes you are going to read, as the second parameter of TinyI2C.start(). With this approach TinyI2C.read() will automatically terminate the last call with a NAK:

TinyI2C.start(Address, 2);
int mins = TinyI2C.read();
int hrs = TinyI2C.read();
TinyI2C.stop();

Alternatively you can just specify the second parameter of TinyI2C.start() as -1, and explicitly identify the last TinyI2C.read command by calling TinyI2C.readlast():

TinyI2C.start(Address, -1);
int mins = TinyI2C.read();
int hrs = TinyI2C.readLast();
TinyI2C.stop();

*/

#endif /* I2C_H_ */