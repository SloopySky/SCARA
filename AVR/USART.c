#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include "USART.h"
#include <util/setbaud.h>

#define   USART_HAS_DATA   bit_is_set(UCSR0A, RXC0)
#define   USART_READY      bit_is_set(UCSR0A, UDRE0)


void init_USART(void) {
	// Set BAUD rate:
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

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
	loop_until_bit_is_set(UCSR0A, UDRE0);
        // Send data:
	UDR0 = data;
}

uint8_t receive_byte(void) {
	// Wait for incoming data:
	loop_until_bit_is_set(UCSR0A, RXC0);
	// Return register value:
	return UDR0;
}

void print_string(const char str[]) {
	uint8_t i = 0;
	while (str[i]) {
		transmitByte(str[i]);
		i++;
	}
}

void read_string(char str[], uint8_t max_length) {
	char response;
	uint8_t i = 0;
	// Prevent over-runs:
	while (i < (maxLength - 1)) {
		response = receiveByte();
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

	sprintf(str, "%.*f", precision, number);
	print_string(str);
}

/*
Consider use of interrupts...
*/