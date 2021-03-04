#include <avr/io.h>
#include <stdbool.h>
#include "Time.h"
#include "Port.h"
#include "Stepper.h"

// Max number of steppers
#define MAX_STEPPERS			5

// Default value of field max_speed in deg/s
#define MAX_SPEED_DEFAULT 		50

// Directions ClockWise and CounterClockWise
#define CW		 		 1
#define CCW				-1

/*
bool step() function generates square wave in order to execute steps.
It is realized by changing state of the step_pin of each motor,
after passage of the interval time.
Returns true if at least one motor isn't on its target position,
false if all steppers reached their target positions.
*/
static bool step(void);

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
	bool reverse_direction;		// Set True to reverse direction
	uint32_t steps;				// [steps]
	float deg_per_step;			// [deg/step]
	float max_speed;			// [steps/us]
	float interval;				// [us]
	uint32_t last_step_time;	// [us]
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
	set_speed(g_stepper_count, MAX_SPEED_DEFAULT);

	// Enable outputs
	DDR(steppers[g_stepper_count].port) |= (1 << steppers[g_stepper_count].step_pin) \
					    				|  (1 << steppers[g_stepper_count].dir_pin);

	return(++g_stepper_count);	// Increment and return number of motors initialized so far	
}

uint32_t motion(const float angles[]) {
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
		if (steppers[i].max_speed)
			this_time = steppers[i].steps / steppers[i].max_speed;
		if (this_time > longest_time) longest_time = this_time;
	}
	if (longest_time > 0.0) {
		// Work out a new interval for each stepper so they will all 
		// arrived at the same time of longest_time
		for (i = 0; i < g_stepper_count; i++) {
			if (steppers[i].steps == 0) steppers[i].interval = longest_time;
			else steppers[i].interval = longest_time / (2.0 * steppers[i].steps);
		}
	}
	return longest_time;
}

static bool step(void) {
	bool ret = false;	// Assume that all motors have finished
	for (uint8_t i = 0; i < g_stepper_count; i++) {
		if (steppers[i].steps) {
			// There is at least one step to be performed
			// Read current time in microseconds	
			uint32_t time = micros();	
    			if (time - steppers[i].last_step_time >= steppers[i].interval) {
				// It's time to step
				// Generate square wave
				// If step pin state is high set it low
				if (PORT(steppers[i].port) & (1 << steppers[i].step_pin)) {
					PORT(steppers[i].port) &= ~(1 << steppers[i].step_pin);
					steppers[i].steps--;	// Count steps on trailing edge
				}
				// If step pin state is low set it high
				else PORT(steppers[i].port) |= (1 << steppers[i].step_pin);
				// Remember time of this step
				steppers[i].last_step_time = time;	 
			}
			ret = true;	// At least one motor hasn't finished yet
		}
	}
	// Return true if at least one motor hasn't finished yet, otherwise false
	return ret;
}

void run(void) {
	while(step());	// Run motors until all of them arrive
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
	if (g_stepper_count < index) return;		// Such stepper not initialized, exit function
	steppers[index].reverse_direction = reverse;	// Reverse directions of specified motor
}

void set_stepping(uint8_t index, uint8_t stepping) {
	if (g_stepper_count < index) return;		// Such stepper not initialized, exit function
	if (stepping < 1) stepping = 1;				// Minimum stepping value is 1
	
	// Recalculate degrees per one step:
	steppers[index].deg_per_step = steppers[index].deg_per_step / stepping;
	
	// Recalculate speed:
	set_speed(index, MAX_SPEED_DEFAULT);
}

void set_speed(uint8_t index, float speed) { 			// speed in deg/s
	if (g_stepper_count < index) return;				// Such stepper not initialized, exit function	
	if (speed < 0) steppers[index].max_speed = 0.0;		// Speed can't be nagative
	
	// Convert deg/s to steps/us and store value:
	else steppers[index].max_speed = speed / (1000000.0 * steppers[index].deg_per_step);
}
