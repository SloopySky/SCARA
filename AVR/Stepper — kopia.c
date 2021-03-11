#include <avr/io.h>
#include <stdbool.h>
#include "Time.h"
#include "Port.h"
#include "Stepper.h"

// Max number of steppers
#define MAX_STEPPERS			5

// Default value of field max_speed in rpm
#define DEFAULT_SPEED			50

// Directions ClockWise and CounterClockWise
#define CW		 		 1
#define CCW				-1

/*
square_wave() function generates square wave in order to execute steps.
It is realized by changing state of the step_pin of each motor.
*/
static void square_wave(uint8_t index);

/*
set_direction() function sets the dir_pin state of the particular motor
to value that corresponds with the dir argument (CW or CCW).
Arguments:
* index - number of stepper motor
* dir - rotation direction (CW or CCW)
*/
static void set_direction(uint8_t index, int8_t dir);

struct Stepper {
	volatile uint8_t *port;
	uint8_t step_pin;
	uint8_t dir_pin;
	bool reverse_direction;			// Set True to reverse direction
	uint32_t steps;				// [steps]
	float deg_per_step;			// [deg/step]
	float speed;				// [steps/us]
	float interval;				// [us]
	uint32_t last_step_time;		// [us]
} steppers[MAX_STEPPERS];

static uint8_t g_stepper_count = 0;

uint8_t add_stepper(volatile uint8_t *port, uint8_t step_pin, uint8_t dir_pin, float deg_per_step) {
	if (!g_stepper_count) init_time();
	
	if (deg_per_step <= 0.0) deg_per_step = 1.0;		// deg_per_step is a physical factor, 
														// must be greater than 0 
	steppers[g_stepper_count].port = port;		
	steppers[g_stepper_count].step_pin = step_pin;
	steppers[g_stepper_count].dir_pin = dir_pin;
	steppers[g_stepper_count].deg_per_step = deg_per_step;
	steppers[g_stepper_count].reverse_direction = 0;	// Direction not reversed as default
	
	// Set default speed 
	set_speed(g_stepper_count, DEFAULT_SPEED);

	// Enable outputs
	DDR(steppers[g_stepper_count].port) |= (1 << steppers[g_stepper_count].step_pin) \
					    				|  (1 << steppers[g_stepper_count].dir_pin);

	return(++g_stepper_count);	// Increment and return number of motors initialized so far	
}

uint32_t prepare_simultanuous(const float angles[]) {
	float longest_time = 0.0;
	uint8_t i;
	for (i = 0; i < g_stepper_count; i++) {
		if (angles[i] >= 0) {
			set_direction(i, CW);
			steppers[i].steps = angles[i] / steppers[i].deg_per_step;	
		} 
		else {
			set_direction(i, CCW);
			steppers[i].steps = -angles[i] / steppers[i].deg_per_step;
		}

		float this_time = 0.0;
		if (steppers[i].speed)
			this_time = steppers[i].steps / steppers[i].speed;
		if (this_time > longest_time) longest_time = this_time;
	}
	if (longest_time > 0.0) {
		// Work out a new interval for each stepper so they will all 
		// arrived at the same time of longest_time
		for (i = 0; i < g_stepper_count; i++) {
			if (!steppers[i].steps) steppers[i].interval = longest_time;
			else steppers[i].interval = longest_time / (2.0 * steppers[i].steps);
		}
	}
	return longest_time;
}

void prepare_single(uint8_t index, float angle) {
	stepper[index].steps = angle / steppers[index].deg_per_step;
	stepper[index].interval = 1.0 / (2.0 * stepper[index].speed);
}

static void square_wave(uint8_t index) {
	// Change step pin state:
	PORT(steppers[index].port) ^= (1 << steppers[index].step_pin);

	// Count steps on trailing edge:
	if (steppers[index].steps > 0)
		if (PORT(steppers[index].port) & (1 << steppers[index].step_pin))
			steppers[index].steps--;
}

void step(uint8_t index) {
	// Read current time in microseconds:
	uint32_t time = micros();	
    	if (time - steppers[index].last_step_time >= steppers[index].interval) {
		// Time to make a step:
		square_wave(index);
		// Remember time of this step
		steppers[index].last_step_time = time;	 
	}
}

void run(void) {
	bool all_arrived = false;
	// Run motors until all of them arrive:
	while(!all_arrived) {
		// Assume that all steppers arrived:
		all_arrived = true;
		for (uint8_t i = 0; i < g_stepper_count; i++) {
			if (steppers[i].steps) {
				// There is at least one step to be performed,
				// At least one motor hasn't finished yet:
				all_arrived = false
				step(i);
			}
		}
	}
}

static void set_direction(uint8_t index, int8_t dir) {
	if (g_stepper_count < index) return;
	if (steppers[index].reverse_direction) dir = -dir;

	if (dir == CW)
		PORT(steppers[index].port) &= ~(1 << steppers[index].dir_pin);
	if (dir == CCW)
		PORT(steppers[index].port) |= (1 << steppers[index].dir_pin);
}

void reverse_direction(uint8_t index, bool reverse) {
	if (g_stepper_count < index) return;			// Such stepper not initialized, exit function
	steppers[index].reverse_direction = reverse;		// Reverse directions of specified motor
}

void set_stepping(uint8_t index, uint8_t stepping) {
	if (g_stepper_count < index) return;			// Such stepper not initialized, exit function
	if (stepping < 1) stepping = 1;				// Minimum stepping value is 1
	
	// Recalculate degrees per one step:
	steppers[index].deg_per_step = steppers[index].deg_per_step / stepping;
	
	// Recalculate speed:
	set_speed(index, DEFAULT_SPEED);
}

void set_speed(uint8_t index, float speed) { 			// speed in rpm
	if (g_stepper_count < index) return;			// Such stepper not initialized, exit function	
	if (speed < 0) steppers[index].speed = 0.0;		// Speed can't be nagative
	if (speed > MAX_SPEED) speed = MAX_SPEED;
	
	// Convert rpm to steps/us and store value:
	else steppers[index].speed = speed * 360.0 / (1000000.0 * steppers[index].deg_per_step);
}
