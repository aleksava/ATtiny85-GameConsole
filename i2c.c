/* TinyI2C v2.0.1
   David Johnson-Davies - www.technoblogy.com - 5th June 2022
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/


#ifndef F_CPU
#define F_CPU 8000000UL
#endif


#include <xc.h>
#include "i2c.h"
//#include <stdbool.h>
#include <util/delay.h>

/* *********************************************************************************************************************
   Minimal Tiny I2C Routines for original ATtiny chips that support I2C using a USI peripheral, such as ATtiny85.
********************************************************************************************************************* */




// Defines
#define TWI_FAST_MODE

#ifdef TWI_FAST_MODE                 // TWI FAST mode timing limits. SCL = 100-400kHz
#define DELAY_T2TWI (_delay_us(2))   // >1.3us
#define DELAY_T4TWI (_delay_us(1))   // >0.6us
#else                                // TWI STANDARD mode timing limits. SCL <= 100kHz
#define DELAY_T2TWI (_delay_us(5))   // >4.7us
#define DELAY_T4TWI (_delay_us(4))   // >4.0us
#endif

#define TWI_NACK_BIT 0 // Bit position for (N)ACK bit.

// Constants
// Prepare register value to: Clear flags, and set USI to shift 8 bits i.e. count 16 clock edges.
unsigned char const USISR_8bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0x0<<USICNT0;
// Prepare register value to: Clear flags, and set USI to shift 1 bit i.e. count 2 clock edges.
unsigned char const USISR_1bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0xE<<USICNT0;


uint16_t I2Ccount;

/* Local API declarations */

uint8_t i2c_transfer (uint8_t data) {
  USISR = data;                                                   // Set USISR according to data.
                                                                  // Prepare clocking.
  data = 0<<USISIE | 0<<USIOIE |                                  // Interrupts disabled
         1<<USIWM1 | 0<<USIWM0 |                                  // Set USI in Two-wire mode.
         1<<USICS1 | 0<<USICS0 | 1<<USICLK |                      // Software clock strobe as source.
         1<<USITC;                                                // Toggle Clock Port.
  do {
    DELAY_T2TWI;
    USICR = data;                                                 // Generate positive SCL edge.
    while (!(PIN_USI_CL & 1<<PIN_USI_SCL));                       // Wait for SCL to go high.
    DELAY_T4TWI;
    USICR = data;                                                 // Generate negative SCL edge.
  } while (!(USISR & 1<<USIOIF));                                 // Check for transfer complete.

  DELAY_T2TWI;
  data = USIDR;                                                 // Read out data.
  USIDR = 0xFF;                                                 // Release SDA.
  DDRB |= (1<<PIN_USI_SDA);										// Enable SDA as output.

  return data;                                                  // Return the data from the USIDR
}


/* Public API declarations */

void i2c_init (void) {
  PORTB |= 1<<PIN_USI_SDA;										// Enable pullup on SDA.
  PORTB |= 1<<PIN_USI_SCL;										// Enable pullup on SCL.

  DDRB |= 1<<PIN_USI_SCL;										// Enable SCL as output.
  DDRB |= 1<<PIN_USI_SDA;										// Enable SDA as output.

  USIDR = 0xFF;                                                   // Preload data register with data.
  USICR = 0<<USISIE | 0<<USIOIE |                                 // Disable Interrupts.
          1<<USIWM1 | 0<<USIWM0 |                                 // Set USI in Two-wire mode.
          1<<USICS1 | 0<<USICS0 | 1<<USICLK |                     // Software stobe as counter clock source
          0<<USITC;
  USISR = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | // Clear flags,
          0x0<<USICNT0;                                           // and reset counter.
}

uint8_t i2c_read (void) {
  if ((I2Ccount != 0) && (I2Ccount != -1)) I2Ccount--;
  
  /* Read a byte */
  DDRB &= ~(1<<PIN_USI_SDA);                                   // Enable SDA as input.
  uint8_t data = i2c_transfer(USISR_8bit);

  /* Prepare to generate ACK (or NACK in case of End Of Transmission) */
  if (I2Ccount == 0) USIDR = 0xFF; else USIDR = 0x00;
  i2c_transfer(USISR_1bit);                            // Generate ACK/NACK.

  return data;                                                    // Read successfully completed
}

uint8_t i2c_readLast (void) {
  I2Ccount = 0;
  return i2c_read();
}

bool i2c_write (uint8_t data) {
  /* Write a byte */
  PORTB &= ~(1<<PIN_USI_SCL);                               // Pull SCL LOW.
  USIDR = data;                                                   // Setup data.
  i2c_transfer(USISR_8bit);                            // Send 8 bits on bus.

  /* Clock and verify (N)ACK from slave */
  DDRB &= ~(1<<PIN_USI_SDA);                                   // Enable SDA as input.
  if (i2c_transfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false;

  return true;                                                    // Write successfully completed
}

// Start transmission by sending address
bool i2c_start (uint8_t address, uint16_t readcount) {
  if (readcount != 0) { I2Ccount = readcount; readcount = 1; }
  uint8_t addressRW = address<<1 | readcount;

  /* Release SCL to ensure that (repeated) Start can be performed */
  PORTB |= 1<<PIN_USI_SCL;                                  // Release SCL.
  while (!(PIN_USI_CL & 1<<PIN_USI_SCL));                         // Verify that SCL becomes high.
#ifdef TWI_FAST_MODE
  DELAY_T4TWI;
#else
  DELAY_T2TWI;
#endif

  /* Generate Start Condition */
  PORTB &= ~(1<<PIN_USI_SDA);                                  // Force SDA LOW.
  DELAY_T4TWI;
  PORTB &= ~(1<<PIN_USI_SCL);								// Pull SCL LOW.
  PORTB |= 1<<PIN_USI_SDA;                                     // Release SDA.

  if (!(USISR & 1<<USISIF)) return false;

  /*Write address */
  PORTB &= ~(1<<PIN_USI_SCL);                               // Pull SCL LOW.
  USIDR = addressRW;                                              // Setup data.
  i2c_transfer(USISR_8bit);                            // Send 8 bits on bus.

  /* Clock and verify (N)ACK from slave */
  DDRB &= ~(1<<PIN_USI_SDA);                                   // Enable SDA as input.
  if (i2c_transfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false; // No ACK

  return true;                                                    // Start successfully completed
}

bool i2c_restart(uint8_t address, uint16_t readcount) {
  return i2c_start(address, readcount);
}

void i2c_stop (void) {
  PORTB &= ~(1<<PIN_USI_SDA);                                  // Pull SDA low.
  PORTB |= 1<<PIN_USI_SCL;                                  // Release SCL.
  while (!(PIN_USI_CL & 1<<PIN_USI_SCL));                         // Wait for SCL to go high.
  DELAY_T4TWI;
  PORTB |= 1<<PIN_USI_SDA;                                     // Release SDA.
  DELAY_T2TWI;
}
