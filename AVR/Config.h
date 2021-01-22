/***** CONNECTIONS TO AVR *****/
#define ENDSTOP				PD2

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


/***** Machine constants and parameters *****/
#define NUMBER_OF_AXES			4
#define NUMBER_OF_STEPPERS		3


/***** NAMES *****/
#define Z				0
#define ARM				1
#define FOREARM				2
#define WRIST
#define GRIPPER


/***** STEPPER MOTORS PARAMETERS *****/
#define STEPPING			4		// 1/4 step	
#define DEG_PER_STEP			1.8		// deg/step	1 step --> 1.8 deg


/***** REDUCTIONS *****/
#define iz 				360/8		//Z Axis reduction 8 mm/rev screw pitch
#define i1 				72.0/16.0	//Arm reduction 
#define i21 				62.0/16.0	//Forearm reduction
#define i22 				62.0/33.0		


/***** LENGTHS *****/
#define l1				93.0		//Arm length
#define l2				106.5		//Forearm length


/***** RANGE OF MOTION OF EACH AXIS *****/
#define ARM_RANGE			190
#define FOREARM_RANGE			280
#define Z_RANGE				300
