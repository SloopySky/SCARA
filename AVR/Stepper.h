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
assigns positions as zero, max_speed as default.
deg_per_step argument is a stepper motor parameter.
stepping argument value depends of physical microstepping resolution as follows: 
1: full step
2: half step
4: quarter step, etc.
At the first call initializes timer that is used to drive steppers.
Returns number of steppers that are initialized so far.
*/
uint8_t add_stepper(volatile uint8_t *port, uint8_t step_pin, uint8_t dir_pin, \
					uint8_t stepping, float deg_per_step);

/*
move_steps() function doesn't perform any motion,
just prepares parameters for the future motion.
Calculates and returns duration time of the motion.
For each stepper:
* sets direction field and PIN to the right values
* calculates interval time between steps and the target position of the movement.
Argument steps[] should contain array of steps to be performed
by corresponding stepper.
*/
uint32_t move_steps(const int32_t steps[]);

/*
move_angles() function calculates amount of steps to perform in order to obtain
angular displacement passed by argument array angles[], then calls move_steps().
Returns duration time of the motion.
*/
uint32_t move_angles(const float angles[])

/*
run() function performs steps
unless all steppers reach their target positions.
*/
void run(void);

/*
set_speed() function assigns the max_speed field
of i-index Stepper struct from steppers array
with function argument speed in deg/s.
*/
void set_speed(uint8_t i, float speed);

#endif

#if 0
EXAMPLE OF USE:

add_stepper(&PORTB, PB6, PB7, 4, 1.0/1.8);	// stepper index = 0	
add_stepper(&PORTB, PB1, PB2, 4, 1.0/1.8);	// stepper index = 1	

float angles[] = {30, 90};			// stepper 0 angle = 30, stepper 1 angle = 90

move_angles(angles);				// calculate
run();						// move

#endif
