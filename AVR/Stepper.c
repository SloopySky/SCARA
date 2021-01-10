#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "Stepper.h"

#define clock_cycles_per_microsecond() (F_CPU/1000000L)
#define abs(x) ((x)<0 ? -(x):(x))
#define PORT(x) (*x)
#define DDR(x) (*(x - 1))
#define MAX_STEPPERS			10

//default value of field max_speed in steps/s
#define MAX_SPEED_DEFAULT 		100

// directions ClockWise and CounterClockWise
#define CW		 				 1
#define CCW						-1

struct Stepper {
	volatile uint8_t *port;
	uint8_t step_pin;
	uint8_t dir_pin;
	int32_t current_position;
	int32_t target_position;
	int8_t direction;
	float max_speed;
	float interval;
	uint32_t last_step_time;
} steppers[MAX_STEPPERS];

static volatile uint32_t timer0_overflow_count = 0;
static uint8_t g_stepper_count = 0;

ISR(TIMER0_OVF_vect) {
	timer0_overflow_count++;
}

static uint32_t micros(void) {
	uint32_t m;
	uint8_t oldSREG = SREG, t;
	cli();
	m = timer0_overflow_count;
	t = TCNT0;
	if ((TIFR0 & (1 << TOV0)) && (t < 255))
		m++;
	SREG = oldSREG;

	return ((m << 8) + t) * (64/clock_cycles_per_microsecond());
}

static void init_stepper(void) {
	TCCR0A = 0;
	TCCR0B |= (1 << CS00) | (1 << CS01);
	TIMSK0 |= (1 << TOIE0);
	TCNT0 = 0;
	sei();
}

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

uint8_t add_stepper(volatile uint8_t *port, const uint8_t step_pin, const uint8_t dir_pin) {
	if (!g_stepper_count) init_stepper();
	
	steppers[g_stepper_count].port = port;
	steppers[g_stepper_count].step_pin = step_pin;
	steppers[g_stepper_count].dir_pin = dir_pin;
	steppers[g_stepper_count].current_position = 0;
	steppers[g_stepper_count].target_position = 0;
	steppers[g_stepper_count].max_speed = MAX_SPEED_DEFAULT;

	//Enable outputs
	DDR(steppers[g_stepper_count].port) |= (1 << steppers[g_stepper_count].step_pin) \
										|  (1 << steppers[g_stepper_count].dir_pin);
	
	return(++g_stepper_count);
}

uint32_t motion(const int32_t displacement[]) {
	float longest_time = 0.0;
	uint8_t i;
	for (i = 0; i < g_stepper_count; i++) {
		float this_time = 0.0;
		if (steppers[i].max_speed)
			this_time = abs(displacement[i])*1000000.0/steppers[i].max_speed;
		if (this_time > longest_time) longest_time = this_time;
	
		if (displacement[i] >= 0) {
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
			if (displacement[i] == 0) steppers[i].interval = longest_time;
			else steppers[i].interval = longest_time/(2.0 * abs(displacement[i]));
			steppers[i].target_position = steppers[i].current_position + displacement[i];
		}
	}
	return longest_time;
}
   
void run(void) {
	while(step());
}

void set_speed(const uint8_t i, const float speed) {
	if (speed < 0) steppers[i].max_speed = 0.0;
	else steppers[i].max_speed = speed;
}	
