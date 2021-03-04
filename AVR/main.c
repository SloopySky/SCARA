#define F_CPU 8000000UL
#define BAUD 38400

#include <avr/io.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdbool.h>
#include "USART.h"
#include "Stepper.h"
#include "Config.h"
#include "Port.h"


// Function prototypes:
bool init_scara(void);
void move(float values[]);
void set_zero(void);
void home(void);
void set_feed(float f);
void wait(float time_s);
void menu(void);


// Global variables:
static bool is_initialized = 0;
static float reductions[NUMBER_OF_AXES];
static float current_position[NUMBER_OF_AXES];


int main(void) {
	clock_prescale_set(clock_div_1);
	init_USART();
	init_scara();
	
	while(1) {
		menu();
	}
}


// Function definitions:
void init_scara(void) {
	reductions[0] = iz;
	reductions[1] = i1;
	reductions[2] = i21*i22;

	// Stepper motors initialization:
	add_stepper(&Z_PORT, Z_STEP, Z_DIR, DEG_PER_STEP);
	add_stepper(&ARM_PORT, ARM_STEP, ARM_DIR, DEG_PER_STEP);
	add_stepper(&FOREARM_PORT, FOREARM_STEP, FOREARM_DIR, DEG_PER_STEP);
	set_stepping(Z, STEPPING);
	set_stepping(ARM, STEPPING);
	set_stepping(FOREARM, STEPPING);

	// Servos initialization:

	// Endstop initialization:
	DDR(&ENDSTOP_PORT) &= ~(1 << ENDSTOP_PIN);

	for (uint8_t i = 0; i < NUMBER_OF_AXES; i++)
		current_position[i] = 0;
	is_initialized = 1;
}

void move(float values[]) {
	// Store reductions values:
	static const float superposition = i22;

	// Recalculate displacement values
	// taking into account reductions:
	for (uint8_t i = 0; i < NUMBER_OF_STEPPERS; i++) {
		float disp = values[i] - current_position[i];
		current_position[i] = values[i];
		values[i] = disp * reductions[i];
	}
	// Forearm and arm motion superposition correction:
	values[FOREARM] += values[ARM]/superposition;
		
	motion(values);
	run();
}

// Z axis homing procedure:
void home_Z(void) {
	float values[NUMBER_OF_STEPPERS];
	for (uint8_t i = 0; i < NUMBER_OF_STEPPERS; i++)
		values[i] = 0;

	// Move Z downwards,
	// until the endstop will trigger:
	while(!((PIN(&ENDSTOP_PORT) & (1 << ENDSTOP_PIN))) {
		// Step size of 0.1 mm:
		values[Z] -= 0.1 * reductions[Z];
		motion(values);
		run();
	}
	current_position[Z] = 0;
}

void feed(float speed) {
	for (uint8_t i = 0; i < NUMBER_OF_STEPPERS; i++)
		set_speed(i, (speed * reductions[i]));
}	

void wait(float time_s) {
	time_s *= 10;
	for (uint8_t i = 0; i < time_s; i++)
		_delay_ms(100);
}

void menu(void) {	
	// Send ready signal:
	transmit_byte('1');	

	// Get function code:
	char function_code = receive_byte();
	receive_byte();
	
	// Skip if not initialized:
	if (!is_initialized) function_code = '0';

	switch (function_code) {
		// Move:
		case 'G':
			float values[NUMBER_OF_AXES];
			for (uint8_t i = 0; i < NUMBER_OF_AXES; i++) {
				values[i] = get_float();
			}
			move(values);
			break;

		// Feed:
		case 'F':
			float feed = get_float();			
			set_feed(feed);	
			break;
		
		// Gripper On/Off:
		case 'M':
			char on_off = receiveByte();
			// if (on_off == '3') // Close the gripper
			// else if (on_off == '4') // Open the gripper
			break;
		
		// Wait:
		case 'W':
			float time_s = get_float();
			wait(time_s);
			break;

		// Home:
		case 'H':
			home_Z();
			break;
	}
	// Flush data that remains in the buffer:
	flush();
}
