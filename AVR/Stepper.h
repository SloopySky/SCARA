// Stepper.h is a library that allows to
// control simultanouesly up to 10 stepper motors 
// ver.: 08.01.2021

#ifndef STEPPER_H
#define STEPPER_H

/*
REQUIRES:
#include <avr/io.h>
#include <stdbool.h>
#include "Time.h"
*/

/*
add_stepper() function initializes a stepper in the array of steppers,
writes addresses of hardware connections that are passed by arguments,
sets default speed.
deg_per_step argument is a stepper motor parameter,
that defines degrees of rotation that is caused by a single step signal.
At the first call initializes timer that is used to drive steppers.
Returns number of steppers that are initialized so far.
*/
uint8_t add_stepper(volatile uint8_t *port, uint8_t step_pin, uint8_t dir_pin, float deg_per_step);

/*
motion() function doesn't perform any motion,
just prepares parameters for the future simultaneous motion
of each motor by the corresponding angle from array that is spassed as argument.
Returns duration time of the motion.
*/
uint32_t motion(const float angles[]);

/*
run() function performs steps
unless all steppers reach their target positions.
*/
void run(void);

/*
reverse_direction() with argument value of True reverses direction of rotation
of the motor specified by index argument, so that CW becomes CCW and vice versa.
False value of argument restores directions. 
*/
void reverse_direction(uint8_t index, bool reverse);

/*
set_stepping() allows to change physical microstepping resolution.
Specific values of stepping argument correspond with the adequate mode as follows: 
1: full step
2: half step
4: quarter step, etc.
*/
void set_stepping(uint8_t index, uint8_t stepping);

/*
set_speed() function stores the speed value (in deg/s) passed as argument
of i-index motor
with function argument speed in deg/s.
*/
void set_speed(uint8_t i, float speed);

#endif

#if 0
EXAMPLE OF USE:

add_stepper(&PORTB, PB6, PB7, 4, 1.8);		// stepper index = 0	
add_stepper(&PORTB, PB1, PB2, 4, 1.8);		// stepper index = 1	

float angles[] = {30, 90};			// stepper 0 angle = 30, stepper 1 angle = 90

motion(angles);					// calculate
run();						// move

#endif
