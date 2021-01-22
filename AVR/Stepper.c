#include <avr/io.h>
#include <stdbool.h>
#include "Time.h"
#include "Stepper.h"

#define abs(x) ((x)<0 ? -(x):(x))
#define PORT(x) (*x)		// Pin state register (HIGH or LOW)
#define DDR(x) (*(x - 1))	// Data Direction Register (input or output)

// Max number of steppers
#define MAX_STEPPERS			10

// Default value of field max_speed in deg/s
#define MAX_SPEED_DEFAULT 		10

// Directions ClockWise and CounterClockWise
#define CW		 		 1
#define CCW				-1

struct Stepper {
	volatile uint8_t *port;
	uint8_t step_pin;
	uint8_t dir_pin;
	float deg_per_step;		// [deg/step]
	int32_t current_position;	// [steps]
	int32_t target_position;	// [steps]
	int8_t direction;		// CW or CCW
	float max_speed;		// [deg/s]
	float interval;			// [us]
	uint32_t last_step_time;	// [us]
} steppers[MAX_STEPPERS];

static uint8_t g_stepper_count = 0;

static bool step(void) {
	bool ret = false;
	for (uint8_t i = 0; i < g_stepper_count; i++) {
		if (steppers[i].current_position != steppers[i].target_position) {
			uint32_t time = micros();   
    			if (time - steppers[i].last_step_time >= steppers[i].interval) {
				if (PORT(steppers[i].port) & (1 << steppers[i].step_pin)) {
					PORT(steppers[i].port) &= ~(1 << steppers[i].step_pin);
					steppers[i].current_position += steppers[i].direction;
				}
				else PORT(steppers[i].port) |= (1 << steppers[i].step_pin);
				steppers[i].last_step_time = time;
			}
			ret = true;
		}
	}
	return ret;
}

uint8_t add_stepper(volatile uint8_t *port, uint8_t step_pin, uint8_t dir_pin, uint8_t stepping, float deg_per_step) {
	if (!g_stepper_count) init_time();
	
	steppers[g_stepper_count].port = port;
	steppers[g_stepper_count].step_pin = step_pin;
	steppers[g_stepper_count].dir_pin = dir_pin;
	steppers[g_stepper_count].deg_per_step = deg_per_step / stepping;
	steppers[g_stepper_count].current_position = 0;
	steppers[g_stepper_count].target_position = 0;
	steppers[g_stepper_count].max_speed = MAX_SPEED_DEFAULT;

	//Enable outputs
	DDR(steppers[g_stepper_count].port) |= (1 << steppers[g_stepper_count].step_pin) \
					    |  (1 << steppers[g_stepper_count].dir_pin);
	
	return(++g_stepper_count);
}

uint32_t move_angles(const float angles[]) {
	static int32_t steps[g_stepper_count];
	for (i = 0; i < g_stepper_count; i++)
		steps[i] = angles[i] / steppers[i].deg_per_step;
	
	return move_steps(steps);
}

uint32_t move_steps(const int32_t steps[]) {
	float longest_time = 0.0;
	uint8_t i;
	for (i = 0; i < g_stepper_count; i++) {
		float this_time = 0.0;
		if (steppers[i].max_speed)
			this_time = abs(steps[i]) * 1000000.0 * steppers[i].deg_per_step / steppers[i].max_speed;
		if (this_time > longest_time) longest_time = this_time;
	
		if (steps[i] >= 0) {
			PORT(steppers[i].port) &= ~(1 << steppers[i].dir_pin);
			steppers[i].direction = CW;
		}
		else {
			PORT(steppers[i].port) |= (1 << steppers[i].dir_pin);
			steppers[i].direction = CCW;
		}
	}

	if (longest_time > 0.0) {
		// Now work out a new max speed for each stepper so they will all 
		// arrived at the same time of longest_time
		for (i = 0; i < g_stepper_count; i++) {
			if (steps[i] == 0) steppers[i].interval = longest_time;
			else steppers[i].interval = longest_time / (2.0 * abs(steps[i]));
			steppers[i].target_position = steppers[i].current_position + steps[i];
		}
	}
	return longest_time;
}
   
void run(void) {
	while(step());
}

void set_speed(uint8_t i, float speed) {
	if (speed < 0) steppers[i].max_speed = 0.0;
	else steppers[i].max_speed = speed;
}	
