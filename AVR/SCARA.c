#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "SCARA.h"
#include "Stepper.h"
#include "Config.h"
#include "USART.h"

#define TABLE_CAPACITY 10

struct Axis {	
	float current_position;
	int16_t min_range;
	int16_t max_range;
	float reduction;
} axis[NUMBER_OF_AXES];

struct Tool {
	char name[20];
	uint8_t nr;
	uint8_t x_offset;
	uint8_t z_offset;
} tools[TABLE_CAPACITY];

struct Point {
	float xyz[3];
	float phi;
} points[TABLE_CAPACITY];


static bool is_initialized = 0;

void init_scara(void) {
	add_stepper(&Z_PORT, Z_STEP, Z_DIR);
	add_stepper(&ARM_PORT, ARM_STEP, ARM_DIR);
	add_stepper(&FOREARM_PORT, FOREARM_STEP, FOREARM_DIR);

	//Z Axis
	axis[Z].reduction = iz;
	axis[Z].min_range = 0;
	axis[Z].max_range = Z_RANGE;
    
	//Arm
	axis[1].reduction = i1;
	axis[1].min_range = -ARM_RANGE;
	axis[1].max_range = ARM_RANGE;
    
	//Forearm
	axis[2].reduction = i21*i22;
	axis[2].min_range = -FOREARM_RANGE;
	axis[2].max_range = FOREARM_RANGE;
	
	printString("\nAxes have to be zeroed.\n");
	printString("Move Arm and Forearm to Zero position, then press any key.");
	getNumber();
	axis[1].current_position = 0;
	axis[2].current_position = 0;
	axis[Z].current_position = 0;
	printString("Done\n");
	
	is_initialized = 1;
}

void move(const float displacement[]) {
	if (!is_initialized) {
		printString("\nNOT INITIALIZED!\n");
		return;
	}
	
	static int32_t steps[NUMBER_OF_STEPPERS];
	
	uint8_t i;
	for(i = 0; i < NUMBER_OF_STEPPERS; i++)			
		if (((axis[i].current_position + displacement[i]) > axis[i].max_range) \
		  | ((axis[i].current_position + displacement[i]) < axis[i].min_range)) {
			printByte(i);
			printString(" Out of range.\n");
			return;
		}
	
	for(i = 0; i < NUMBER_OF_STEPPERS; i++) {	
		axis[i].current_position += displacement[i];	
		steps[i] = displacement[i] * STEPPING * axis[i].reduction * DEG_TO_STEP;
	}	// Saves axis target position and calculates steps to perform
	
	// Forearm and arm motion superposition correction
	float superposition = i22;
	steps[FOREARM] += steps[ARM]/superposition;
		
	motion(steps);
	run();
}

void move_to(const float absolute[]) {
	if (!is_initialized) {
		printString("\nNOT INITIALIZED!\n");
		return;
	}
	
	static float displacement[NUMBER_OF_STEPPERS];
	
	for (uint8_t i = 0; i < NUMBER_OF_STEPPERS; i++) 
		displacement[i] = absolute[i] - axis[i].current_position;
	
	move(displacement);
}

void set_feed(const float f) {	
	if (!is_initialized) {
		printString("\nNOT INITIALIZED!\n");
		return;
	}
	
	float speed = STEPPING * DEG_TO_STEP;
	if (f > 0) speed *= f;
	
	for (uint8_t i = 0; i < NUMBER_OF_STEPPERS; i++) {
		set_speed(i, (speed * axis[i].reduction));
	}
}	

void wait(const uint8_t time_s) {
	for (uint8_t i = 0; i < time_s; i++) _delay_ms(1000);
}



#if 0

void def_point(uint8_t n, const int16_t xyz[], const int8_t phi) {
	static uint8_t counter = 2;
	if (n == 0) {
		printString("\nPoint added on the ");
		printByte(counter);
		printString(" position.");
		n = counter;
	}
	for (uint8_t i = 0; i < 3; i++)	points[n].xyz[i] = xyz[i];
	points[n].phi = phi;
	
	counter++;
}

void gripper_onoff(const uint8_t state){
	
}

void angles_to_xyz(void) {
	xyz[0] = l1*cos(steppers[ARM].pos) + (l2+!is_wrist*tool.x_offset)*cos(steppers[ARM].pos + steppers[FOREARM].pos) + is_wrist*tool.x_offset*cos(steppers[ARM].pos + steppers[FOREARM].pos + wrist.pos);
	xyz[1] = l1*sin(steppers[ARM].pos) + (l2+!is_wrist*tool.x_offset)*sin(steppers[ARM].pos + steppers[FOREARM].pos) + is_wrist*tool.x_offset*sin(steppers[ARM].pos + steppers[FOREARM].pos + wrist.pos);
	xyz[2] = steppers[Z].pos + tool.z_offset;
}	
	
#endif
