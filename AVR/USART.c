#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include "USART.h"

#define BAUD_PRESCALE ((F_CPU / (BAUD * 16UL)) - 1) 

void init_USART(void) {
	// Set BAUD rate:
	UBRR0H = (BAUD_PRESCALE >> 8);
    UBRR0L = BAUD_PRESCALE;  

#if USE_2X
	UCSR0A |= (1 << U2X0);
#else
	UCSR0A &= ~(1 << U2X0);
#endif

	// Enable USART transmitter/receiver:
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	// 8 data bits, 1 stop bit:
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void transmit_byte(uint8_t data) {
	// Wait for empty transmit buffer:
	while (!(UCSR0A & (1<<UDRE)));
	// Send data:
	UDR0 = data;
}

uint8_t receive_byte(void) {
	// Wait for incoming data:
	while (!(UCSR0A & (1<<RXC)));
	// Return register value:
	return UDR0;
}

void flush(void) {
	char trash;
	while (UCSR0A & (1<<RXC)) trash = UDR0;
}

void print_string(const char str[]) {
	uint8_t i = 0;
	while (str[i]) {
		transmit_byte(str[i]);
		i++;
	}
}

void read_string(char str[], uint8_t max_length) {
	char response;
	uint8_t i = 0;
	// Prevent over-runs:
	while (i < (max_length - 1)) {
		response = receive_byte();
		// If 'Enter':
    		if (response == '\r') break;
		else {
			str[i] = response;
			i++;
		}
	}
	// Terminal NULL character:
	str[i] = 0;
}

int get_int(void) {
	char str[LENGTH];

	read_string(str, LENGTH);
	return atoi(str);
}

float get_float(void) {
	char str[LENGTH];

	read_string(str, LENGTH);
	return atof(str);
}

void print_int(int number) {
	char str[LENGTH];

	sprintf(str, "%d", number);
	print_string(str);
}

void print_float(float number, uint8_t precision) {
	char str[LENGTH];

	char *sign = "";
	if (number < 0) {
		sign = "-";
		number = -number;
	}

	// Separate integer and fraction part:
	uint32_t integer_part = number;		// Integer part
	number -= integer_part;				// Fraction part
	
	// Multiply fraction part by 10^precision
	// and cast it to int
	// It'll store only decimals
	// that are specified by precision
	for (uint8_t i = 0; i < precision; i++)
		number *= 10;
	uint32_t float_part = number;
	
	// Round up
	if ((number - float_part) >= 0.5) float_part += 1;

	sprintf(str, "%s%lu.%lu", sign, integer_part, float_part);
	print_string(str);
}

/*
Consider use of interrupts...
*/
