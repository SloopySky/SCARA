// HARDWARE CONFIGURATION

// Connections to AVR:
#define ENDSTOP_PORT			PORTD
#define ENDSTOP_PIN			PD2

#define Z_PORT				PORTB
#define Z_STEP				PB6
#define Z_DIR				PB7

#define ARM_PORT			PORTB
#define ARM_STEP			PB1	
#define ARM_DIR				PB2

#define FOREARM_PORT			PORTB
#define FOREARM_STEP			PB3
#define FOREARM_DIR			PB4

#define WRIST_SERVO			PD6
#define T_SERVO				PD5


// Machine constants and parameters:
#define NUMBER_OF_AXES			4
#define NUMBER_OF_STEPPERS		3


// Names:
#define Z				0
#define ARM				1
#define FOREARM				2
#define WRIST
#define GRIPPER


// Stepper motors parameters:
#define STEPPING			4		// 1/4 step	
#define DEG_PER_STEP			1.8		// deg/step	1 step --> 1.8 deg