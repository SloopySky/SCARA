/***** CONNECTIONS TO AVR *****/

//							PIN:
#define DISABLE				PD7
#define ENDSTOP				PD2
#define Z_STEP				PB6
#define Z_DIR				PB7
#define Z_PORT				PORTB
#define ARM_STEP			PB1	
#define ARM_DIR				PB2
#define ARM_PORT			PORTB
#define FOREARM_STEP		PB3
#define FOREARM_DIR			PB4
#define FOREARM_PORT		PORTB
#define WRIST_SERVO			PD6
#define T_SERVO				PD5


/***** Machine constants and parameters *****/
#define NUMBER_OF_AXES			4
#define NUMBER_OF_STEPPERS		3

//Names
#define Z						0
#define ARM						1
#define FOREARM					2
#define WRIST

//Motor parameters:
#define STEPPING				4			// 1/4 step	
#define DEG_TO_STEP				1.0/1.8		// step/deg		1 step --> 1.8 deg

//Reductions:
#define iz 360/8							//Z Axis reduction 8 mm/rev screw pitch
#define i1 72.0/16.0						//Arm reduction 
#define i21 62.0/16.0						//Forearm reduction
#define i22 62.0/33.0		

//Lengths:
#define l1	93								//Arm length
#define l2	106.5							//Forearm length

//Range of motion per axis: 
#define ARM_RANGE				190
#define FOREARM_RANGE			280
#define Z_RANGE					300

//RIGHT- OR LEFT- HANDED
#define LEFT					0
#define	RIGHT					1

//AXIS LINEAR OR ANGULAR
#define LINEAR					0
#define ANGULAR 				1
