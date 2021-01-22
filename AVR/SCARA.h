// SCARA.h is a module that implements
// environment of the SCARA Robot.
// Kinematics of the Robot, machine constants, parameters, etc.
// has to be determined and delivered in Config.h file. 
// ver.: 08.01.2021

#ifndef SCARA_H
#define SCARA_H

/*
REQUIRES:
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include "Config.h"
#include "Stepper.h"	
#include "USART.h"
*/

/*
init_scara() function initializes SCARA module,
delivers stepper motors parameters to the Stepper module,
initializes the gripper, set data to initial values.
Sets static bool g_is_initialized as TRUE.
*/
void init_scara(void);

/*
*/
void move(const float displacement[]);

/*
set_feed() function allows to set velocity of motion.
Calculates velocity for each axis from the f parameter,
taking into account ratios of mechanical transmissions.
*/
void set_feed(const float f);

/*
wait() function produces delay of time 
passed in seconds by time_s argument
*/
void wait(const uint8_t time_s);

/*
gripper_onoff() function opens (state == 1),
closes (state == 0) the gripper,
or changes its current state to the opposite (state == -1)
*/
void gripper_onoff(const uint8_t state);


#endif
