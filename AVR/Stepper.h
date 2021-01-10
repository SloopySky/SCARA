// Stepper.h is a library that allows to
// control simultanouesly up to 10 stepper motors 
// ver.: 08.01.2021

#ifndef STEPPER_H
#define STEPPER_H

/*
REQUIRES:
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
*/

/*
add_stepper() function initializes a stepper in the array of steppers,
writes addresses of hardware connections that are passed by arguments,
assigns positions as zero, max_speed as default.
At the first call initializes timer that is used to drive steppers.
Returns index of stepper currently initialized.
*/
uint8_t add_stepper(volatile uint8_t *port, uint8_t step, uint8_t dir);

/*
motion() function doesn't perform any motion,
just prepares parameters for the future motion.
Calculates and returns duration time of the motion.
For each stepper:
* sets direction field and PIN to the right values
* calculates interval time between steps and the target position of the movement.
Argument displacement[] should contain array of steps to be performed
by corresponding stepper.
*/
uint32_t motion(const int32_t displacement[]);

/*
run() function performs steps
unless all steppers reach their target positions.
*/
void run(void);

/*
set_speed() function assigns the max_speed field
of i-index Stepper struct from steppers array
with function argument speed in steps/s.
*/
void set_speed(const uint8_t i, const float speed);

#endif


/* TO_DOs
** documentation
** velocity control in order to allow linear and circular motions
*/
