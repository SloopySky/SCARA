#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include "Config.h"
#include "Stepper.h"
#include "USART.h"


// Function prototypes:
bool init_scara(void);
void move(float displacement[]);
void set_feed(float f);
void wait(float time_s);
void menu(void);


// Global variables:
static bool is_initialized = 0;
static const float reductions = {iz, i1, i21*i22};


// Main function:
int main(void) {
	init_USART();
	init_scara();

	while(1) {
		menu();
	}
}


// Functions definitions:
bool init_scara(void) {
	if (is_initialized) return 0;

	add_stepper(&Z_PORT, Z_STEP, Z_DIR, STEPS_PER_DEG);
	add_stepper(&ARM_PORT, ARM_STEP, ARM_DIR, STEPS_PER_DEG);
	add_stepper(&FOREARM_PORT, FOREARM_STEP, FOREARM_DIR, STEPS_PER_DEG);
	
	set_stepping(Z, STEPPING);
	set_stepping(ARM, STEPPING);
	set_stepping(FOREARM, STEPPING);

	is_initialized = 1;
	return 1;
}

void move(float angles[]) {
	if (!is_initialized) return;
	
	// Store reductions values
	static const float superposition = i22;

	// Recalculate displacements values
	// taking into account reductions 
	for (uint8_t i = 0; i < NUMBER_OF_STEPPERS; i++)
		angles[i] = angles[i] * reductions[i];
	
	// Forearm and arm motion superposition correction
	angles[FOREARM] += angles[ARM]/superposition;
		
	move_angles(angles);
	run();
}

void feed(float speed) {	
	if (!is_initialized) return;
	
	for (uint8_t i = 0; i < NUMBER_OF_STEPPERS; i++)
		set_speed(i, (speed * reductions[i]));
}	

void wait(float time_s) {
	time_s *= 10;
	for (uint8_t i = 0; i < time_s; i++) _delay_ms(100);
}

void menu(void) {
	// Receive function code:
	char function_code = receive_byte();
	
	// Function codes used:
	// G - move
	// F - set feed
	// M - open or close the gripper

	switch (function_code) {
		case 'G':
			float angles[NUMBER_OF_AXES];
			for (uint8_t i = 0; i < NUMBER_OF_AXES; i++) {
				angles[i] = get_float(); 
				transmitByte(1); // Ready
			}
			move(angles);
			break;

		case 'F':
			float feed = get_float();			
			set_feed(feed);
			break;

		case 'M':
			char on_off = receiveByte();
			// if (on_off == '3') // Close the gripper
			// else if (on_off == '4') // Open the gripper
			break;

		case 'W':
			float time_s = get_float();
			wait(time_s);
			break;
	}
}