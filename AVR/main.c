#define F_CPU 8000000UL
#define BAUD 38400

#include <avr/io.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdbool.h>
#include "USART.h"
#include "Stepper.h"
#include "Config.h"


// Function prototypes:
bool init_scara(void);
void move(float displacement[]);
void set_feed(float f);
void wait(float time_s);
void menu(void);


// Global variables:
static bool is_initialized = 0;
static float reductions[3];


int main(void) {
	clock_prescale_set(clock_div_1);
	init_USART();
	init_scara();
	
	while(1) {
		menu();
	}
}


// Functions definitions:
bool init_scara(void) {
	if (is_initialized) return 0;
	
	reductions[0] = iz;
	reductions[1] = i1;
	reductions[2] = i21*i22;

	add_stepper(&Z_PORT, Z_STEP, Z_DIR, DEG_PER_STEP);
	add_stepper(&ARM_PORT, ARM_STEP, ARM_DIR, DEG_PER_STEP);
	add_stepper(&FOREARM_PORT, FOREARM_STEP, FOREARM_DIR, DEG_PER_STEP);
	
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
		
	motion(angles);
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
	char line[40];
					
	transmit_byte('1'); // Ready
	char function_code = receive_byte();
	
	switch (function_code) {
		case 'G':
			// print_string("Move");
			receive_byte();
			print_string("\n");		
						
			float angles[NUMBER_OF_AXES];
			read_string(line, 40);
			move(angles);
				
			break;

		case 'F':
			print_string("Set feed: ");
			receive_byte();
				
			float feed = get_float();			
			// set_feed(feed);
				
			print_string("Feed: ");
			print_float(feed, 2);
				
			break;

		case 'M':
			print_string("Gripper");
			/*
			char on_off = receiveByte();
			// if (on_off == '3') // Close the gripper
			// else if (on_off == '4') // Open the gripper
			*/
			break;

		case 'W':
			print_string("Wait");
			/*
			float time_s = get_float();
			wait(time_s);
			*/
			break;
	}
}
