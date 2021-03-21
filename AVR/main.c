#define F_CPU 8000000UL
#define BAUD 38400

#include <avr/io.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "USART.h"
#include "Stepper.h"
#include "Config.h"
#include "Port.h"


// Function prototypes:
void init_scara(void);
void set_feed(float feed[]);
void wait(float time_s);
void menu(void);


// Global variables:
static uint8_t is_initialized = 0;


// Z Axis Endstop inerrupt:
ISR(INT0_vect) {
	if (is_initialized) {
		// Stop Z axis:	
		prepare_single(Z, 0);
	}
}


// Main:
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
	// Stepper motors initialization:
	add_stepper(&Z_PORT, Z_STEP, Z_DIR, DEG_PER_STEP);
	add_stepper(&ARM_PORT, ARM_STEP, ARM_DIR, DEG_PER_STEP);
	add_stepper(&FOREARM_PORT, FOREARM_STEP, FOREARM_DIR, DEG_PER_STEP);
	set_stepping(Z, STEPPING);
	set_stepping(ARM, STEPPING);
	set_stepping(FOREARM, STEPPING);

	// Servos initialization:

	// Endstop initialization:
	DDRD &= ~(1 << PD2);	// PD2 as input
	PORTD |= (1 << PD2);	// Pullup resistor enabled
	
	EIMSK |= (1 << INT0);
	EICRA |= (1 << ISC01);	// Interrupt on INT0 falling edge
	sei();					// Interrupts enabled


	is_initialized = 1;
}

void move(float displacement[]) {
	prepare_simultanuous(displacement);
	run();
}

void set_feed(float feed[]) {
	for (uint8_t i = 0; i < NUMBER_OF_STEPPERS; i++)
		set_speed(i, feed[i]);
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
		case 'G': {
			float displacement[NUMBER_OF_AXES];
			for (uint8_t i = 0; i < NUMBER_OF_AXES; i++)
				displacement[i] = get_float();
			move(displacement);
			break;
		}
			
		// Feed:
		case 'F': {
			float feed[NUMBER_OF_AXES];
			for (uint8_t i = 0; i < NUMBER_OF_AXES; i++)
				feed[i] = get_float();			
			set_feed(feed);	
			break;
		}
		
		// Gripper On/Off:
		case 'M': {
			// char on_off = receive_byte();
			// if (on_off == '3') // Close the gripper
			// else if (on_off == '4') // Open the gripper
			break;
		}
		
		// Wait:
		case 'W': {
			float time_s = get_float();
			wait(time_s);
			break;
		}
	}
	// Flush data that remains in the buffer:
	flush();
}
