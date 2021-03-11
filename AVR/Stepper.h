// Stepper.h is a library that allows to
// control simultanouesly many stepper motors
// with a stepper driver, such as A4988 or similar
// ver.: 08.01.2021

#ifndef STEPPER_H
#define STEPPER_H

#ifndef _STDBOOL_H
#include <stdbool.h>
#endif

/*
REQUIRES:
#include <avr/io.h>
#include "Time.h"
#include "Port.h"
*/

#define MAX_SPEED 1200 // Max speed of the motor [rpm]

/*
add_stepper() function initializes a stepper motor,
writes addresses of hardware connections, sets default speed.
At the first call initializes timer that is used to drive steppers.
Arguments:
* port - address of physical port, pins are connected to (&PORTB, &PORTC, &PORTD)
* step_pin - name of pin to pass signal to make a step (e.g. PB1, PD6, etc.)
* dir_pin - name of pin that determines the direction of rotation (e.g. PB1, PD6, etc.)
* deg_per_step - defines degrees of rotation that is caused by a single step signal
Returns: number of steppers that are initialized so far
*/
uint8_t add_stepper(volatile uint8_t *port, uint8_t step_pin, uint8_t dir_pin, float deg_per_step);

/*
motion() function doesn't perform any motion,
just prepares parameters for the future simultaneous motion of each motor
Arguments:
* angles[] - array of angular displacement for corresponding motor
Returns: duration time of the motion
*/
uint32_t motion(const float angles[]);

/*
run() function performs steps
unless all steppers reach their target positions.
*/
void run(void);

/*
reverse_direction() function changes CW and CCW directions of the particular motor
For example, direction should be reversed to keep CW and CCW 
after placing the motor upside down.
Arguments:
* index - number of stepper motor
* reverse - True replaces directions, False restores
*/
void reverse_direction(uint8_t index, bool reverse);

/*
set_stepping() allows to change physical microstepping resolution
Arguments:
* index - number of stepper motor
* stepping - specifies microstepping mode as follows:
  1: full step
  2: half step
  4: quarter step, etc.
*/
void set_stepping(uint8_t index, uint8_t stepping);

/*
set_speed() function stores the speed value for the particular motor
Arguments:
* index - number of stepper motor
* speed - speed value in deg/s
*/
void set_speed(uint8_t index, float speed);

#endif

#if 0
EXAMPLE OF USE:

add_stepper(&PORTB, PB6, PB7, 1.8);		// stepper index = 0	
add_stepper(&PORTB, PB1, PB2, 1.8);		// stepper index = 1	

float angles[] = {30, 90};			// stepper 0 angle = 30, stepper 1 angle = 90

motion(angles);					// calculate
run();						// move

#endif
