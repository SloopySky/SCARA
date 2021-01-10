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
#include <avr/interrupt.h>
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
Contains procedure of homing axes.
Sets static bool g_is_initialized as TRUE.
*/
void init_scara(void);

/*
*/
void move(const float displacement[]);

/*
move_to() function moves the robot to the absolute position.
It calculates displacements from absolute positions 
and then calls move() function.
*/
void move_to(const float absolute[]);

/*
set_feed() function allows to set velocity of motion.
Calculates velocity for each axis from the f parameter,
taking into account ratios of mechanical transmissions.
*/
void set_feed(const float f);

/*
new_point() function allows to add new or edit existing point,
on the i-position of the points array.
Stores cartesian positions (xyz) and Wrist angle (phi).
*/
void new_point(const uint8_t n, const int16_t xyz[], const int8_t phi);

/*
change_zero_point() function activates zero point
that is stored as i-element of the points array.
*/
void change_zero_point(const uint8_t n);

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

/*
tool_change() function sets as active tool that is stored
on i-position of the tools array.
Position 0 means that there is no tool installed currently.
Gripper is stored on the first position of the tools array.
*/
void tool_change(uint8_t n);


#endif


/*
void set_hand(uint8_t hand);
void tool_def(void);
float * xyz_to_angles(const float input[]);
float * angles_to_xyz(const float input[]);
*/


/* TO_DOs
** home procedure
** functions
*/
