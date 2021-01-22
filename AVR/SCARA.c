#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include "SCARA.h"
#include "Stepper.h"
#include "Config.h"
#include "USART.h"

struct Axis {	
	float current_position;
	float reduction;
} axis[NUMBER_OF_STEPPERS];

static bool is_initialized = 0;

void init_scara(void) {
	add_stepper(&Z_PORT, Z_STEP, Z_DIR, STEPPING, STEPS_PER_DEG);
	add_stepper(&ARM_PORT, ARM_STEP, ARM_DIR, STEPPING, STEPS_PER_DEG);
	add_stepper(&FOREARM_PORT, FOREARM_STEP, FOREARM_DIR, STEPPING, STEPS_PER_DEG);

	axis[1].reduction = i1;
	axis[2].reduction = i21*i22;
	axis[Z].reduction = iz;
	axis[1].current_position = 0;
	axis[2].current_position = 0;
	axis[Z].current_position = 0;

	is_initialized = 1;
	printString("\nINITIALIZED!\n");
}

void move(const float displacement[]) {
	if (!is_initialized) {
		printString("\nNOT INITIALIZED!\n");
		return;
	}
	
	static int32_t angles[NUMBER_OF_STEPPERS];
	
	uint8_t i;
	for(i = 0; i < NUMBER_OF_STEPPERS; i++)	{		
		if (((axis[i].current_position + displacement[i]) > axis[i].max_range) \
		  | ((axis[i].current_position + displacement[i]) < axis[i].min_range)) {
			printByte(i);
			printString(" Out of range.\n");
			return;
		}
	}

	for(i = 0; i < NUMBER_OF_STEPPERS; i++) {	
		axis[i].current_position += displacement[i];	
		angles[i] = displacement[i] * axis[i].reduction;
	}	// Saves axis target position and calculates steps to perform
	
	// Forearm and arm motion superposition correction
	float superposition = i22;
	angles[FOREARM] += angles[ARM]/superposition;
		
	move_angles(angles);
	run();
}

void set_feed(float f) {	
	if (!is_initialized) {
		printString("\nNOT INITIALIZED!\n");
		return;
	}
	for (uint8_t i = 0; i < NUMBER_OF_STEPPERS; i++)
		set_speed(i, (f * axis[i].reduction));
}	

void wait(uint8_t time_s) {
	for (uint8_t i = 0; i < time_s; i++) _delay_ms(1000);
}
