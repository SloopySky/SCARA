// USART.h delivers functions to initialize USART,
// send and receive data over USART
// ver.: 17.02.2021

#ifndef USART_H
#define USART_H

/*
REQUIRES:
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/setbaud.h>
*/

// BAUD required to calculate the bit-rate multiplier:
#ifndef BAUD		// If not defined in Makefile...
#define BAUD  9600	// 9600 is a reasonable default
#endif

// Length of string to print or read
#define LENGTH 21

/* 
Takes the defined BAUD and F_CPU,
calculates the bit-clock multiplier,
and configures the hardware USART
*/
void init_USART(void);

/*
Sends data byte to USART
*/
void transmit_byte(uint8_t data);

/*
Waits until any byte of data comes through
If so, receives it and returns
*/
uint8_t receive_byte(void);

/*
Flushes data that remains in the receive buffer
*/
void flush(void);

/*
Sends char array str[] char by char to USART,
until '\r' char (Enter) is received or max_length is reached
*/
void print_string(const char str[]);

/*
Reads single chars from USART and store them in char array str[],
until zero is encountered
*/
void read_string(char str[], uint8_t max_length);

/*
Reads string from USART by read_string() function
Then converts it to float type and returns.
*/
float get_float(void);

/*
Converts number of type int, passed by argument, to char array
and send each char via USART
Maximum length of char array is defined as LENGTH macro
*/
void print_int(int number);

/*
Converts number of type float, passed by argument, to char array
and send each char via USART
Precision argument specifies decimal places
Maximum length of char array is defined as LENGTH macro
*/
void print_float(float number, uint8_t precision);

#endif
