#include <avr/io.h>
#include <avr/interrupt.h>
#include "Time.h"

#define clock_cycles_per_microsecond() (F_CPU/1000000L)

static volatile uint32_t timer0_overflow_count = 0;

ISR(TIMER0_OVF_vect) {				// If TC0 overflow occures
	timer0_overflow_count++;		// Increment counter value
}

uint32_t micros(void) {
	uint32_t m;
	uint8_t oldSREG = SREG, t;
	cli();					// Interrupts disabled
	m = timer0_overflow_count;
	t = TCNT0;
	// Check if an overflow might occur:
	if ((TIFR0 & (1 << TOV0)) && (t < 255)) m++;
	SREG = oldSREG;				// Interrupts enabled

	// Calculate and return time in microseconds
	// (m << 8): timer0_overflow_count * 256 (one overflow for each 256 bits)
	// ((m << 8) + t): total counter value
	// multiplied by 64 to take into account the prescaler, 
	// so we get amount of clock cycles
	// divided by clock_cycles_per_microsecond(),
	// so we get time in microseconds
	return ((m << 8) + t) * (64/clock_cycles_per_microsecond());
}

void init_time(void) {				// Initialize 8-bit TC0
	static uint8_t is_initialized = 0;
	if (is_initialized) return;		// Don't allow to init more than once
	TCCR0A = 0;				// Normal mode
	TCCR0B |= (1 << CS00) | (1 << CS01);	// Set the prescaler to 64
	TIMSK0 |= (1 << TOIE0); 		// Overflow interrupt enable
	TCNT0 = 0;				// Set counter value to 0
	sei();					// Enable interrupt
	is_initialized = 1;			// Initialization done
}

void reset_time(void) {
	uint8_t oldSREG = SREG;
	cli();
	timer0_overflow_count = 0;
	TCNT0 = 0;
	SREG = oldSREG;
}