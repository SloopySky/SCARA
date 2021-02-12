#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include "Config.h"
#include "Stepper.h"
#include "USART.h"


// Functions prototypes:
uint8_t init_scara(void);
void move(const float displacement[]);
void set_feed(float f);
void wait(uint8_t time_s);
bool menu(void);


// Global variables:
static bool is_initialized = 0;
static const float reductions = {iz, i1, i21*i22};

// Main function:
int main(void) {
	initUSART();
	init_scara();

	while(1) {
	}
}


// Functions definitions:
uint8_t init_scara(void) {
	if (is_initialized) return 0;

	add_stepper(&Z_PORT, Z_STEP, Z_DIR, STEPS_PER_DEG);
	add_stepper(&ARM_PORT, ARM_STEP, ARM_DIR, STEPS_PER_DEG);
	add_stepper(&FOREARM_PORT, FOREARM_STEP, FOREARM_DIR, STEPS_PER_DEG);
	
	set_stepping(Z, STEPPING);
	set_stepping(ARM, STEPPING);
	set_stepping(FOREARM, STEPPING);

	is_initialized = 1;
	return is_initialized;
}

void move(const float displacement[]) {
	if (!is_initialized) return;
	
	// Store reductions values
	static const float superposition = i22;

	// Recalculate displacements values
	// taking into account reductions 
	static int32_t angles[NUMBER_OF_STEPPERS];
	for (uint8_t i = 0; i < NUMBER_OF_STEPPERS; i++)
		angles[i] = displacement[i] * reductions[i];
	
	// Forearm and arm motion superposition correction
	angles[FOREARM] += angles[ARM]/superposition;
		
	move_angles(angles);
	run();
}

void set_feed(float speed) {	
	if (!is_initialized) return;

	for (uint8_t i = 0; i < NUMBER_OF_STEPPERS; i++)
		set_speed(i, (speed * reductions[i]));
}	

void wait(uint8_t time_s) {
	for (uint8_t i = 0; i < time_s; i++) _delay_ms(1000);
}

bool menu(void) {

	// receive char funtion_id

	switch (function_id) {
		// G0 - move
		case '0':	
			static float angles[4];
			for (uint8_t i=0, i<4; i++)
				// receive angles[i]
			move(angles);	// from Stepper.h
			break;

		// G4 - wait
		case '4':
			// receive uint8_t time
			// pass it as argument to
			wait(time);
			break;

		// M3 - open the gripper
		case '3':
			// open the gripper
			break;

		// M5 - close the gripper
		case '5':
			// close the gripper
			break;

		// F - set feed
		case 'F':
			// receive integer part of feed
			// receive floating part of feed
			// put them together as float feed
			set_feed(feed);
		break;
		
		default:
			return 0;
			break;
	}
}