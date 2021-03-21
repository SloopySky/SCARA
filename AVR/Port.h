#define PORT(x) (*x)		// Pin state write (HIGH or LOW)
#define DDR(x) (*(x - 1))	// Data Direction Register (input or output)
#define PIN(x) (*(x - 2))	// Pin state read (HIGH or LOW)