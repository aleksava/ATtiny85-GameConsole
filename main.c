/*
 * main.c
 *
 * Created: 8/4/2022 4:51:59 PM
 *  Author: aleks
 */ 

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <xc.h>

#include <stdbool.h>
#include <stdint.h>

#include "avr/io.h"
#include "avr/interrupt.h"
#include "avr/pgmspace.h"
#include "util/delay.h"

#include "i2c.h"
#include "OLED.h"

// F_CPU = 8 MHz




/* Set ut timer1 to toggle every 500ms and output on PB1 */
void timer1_init(void) {
	MCUCR &= ~(1 << PUD);

	// Clear on Compare | Toggle OC1A on compare | PCK/16384 (64 ticks/s)
	TCCR1 |= (1 << CTC1) | (1 << COM1A0) | (1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10);

	OCR1A = 0xFF;

	DDRB |= (1 << PB1);
}

void timer0_init(void) {
	// Output on compare : off | WGM : off
	TCCR0A = 0;

	// Clock select prescaler : clk/1024 = 7812
	TCCR0B |=   (1 << CS02) |
	(0 << CS01) |
	(1 << CS00);

	TCNT0=0;

	// Compre A register
	OCR0A = 0xFE;
}

/* Set up ADC2 as input for PB4, Vref = Vcc */
void adc1_init()
{
	timer0_init();
	// Voltage ref = Vcc |Left adjust | Mux = 0x02, input adc1 = PB4.
	ADMUX |=    (1 << ADLAR) |
	(0 << MUX0)  |
	(1 << MUX1) ;

	// Autotrigger on Timer0 OVF | Digital input disable for ADC2
	ADCSRB |=   (1 << ADTS2) |
	(0 << ADTS1) |
	(0 << ADTS0) |
	(1 << ADC2D);

	// Enable adc adc | Enable auto trigger |prescaler = 64 | Interrupt enable | Enable start conversion
	ADCSRA |=   (1 << ADEN) |
	(1 << ADATE) |
	(1 << ADPS1) |
	(1 << ADPS2) |
	(1 << ADIE) |
	(1 << ADSC);


}


uint8_t counter = 0;
uint8_t adcVal = 0;

ISR(ADC_vect)
{
	counter++;
	adcVal = ADCH;
	TIFR |= (1 << TOV0); // Clear timer0 OVF
}

int main()
{
	i2c_init();
	OLED_init();
	timer1_init();
	adc1_init();
	sei();

	DDRB |= (1 << PB3); // Set PB3 as output
	PINB |= (1 << PB3);
	
	

	
	
	while(1)
	{
		// ADCSRA |= (1 << ADSC); // Start conversion
		// while(ADCSRA & (1 << ADSC)); // Conversion complete

		if (adcVal > 128) {
			PORTB |= (1 << PB3); // Set PB3 high
			
			OLED_fillscreen(0x00);
		}
		else{
			PORTB &= ~(1 << PB3);	// Set PB3 low
			OLED_fillscreen(0xFF);
		}
		_delay_ms(50);
	}
	return 0;
}