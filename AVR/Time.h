// Time.h is a module used for real time counting
// ver.: 14.01.2021

#ifndef TIME_H
#define TIME_H

/*
REQUIRES:
#include <avr/io.h>
#include <avr/interrupt.h>
*/

/*
init_time() function must be run to allow use of this module.
It's protected to be run only once.
*/
void init_time(void);

/*
micros() function returns the number of microseconds 
since the current program began to run. 
This number will overflow after approximately 70 minutes. 
*/
uint32_t micros(void);

/*
reset_time() function sets time counter value to zero.
*/
void reset_time(void);
#endif