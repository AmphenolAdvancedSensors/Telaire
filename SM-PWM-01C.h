
// Assign your channel in pins
#define PM2_IN_PIN 7  //input for PM2 signal, P1


// Assign your channel out pins - built in LED is 13
#define PM2_OUT_PIN 13

// These bit flags are set in bUpdateFlagsShared to indicate which
// channels have new signals
#define PM2_FLAG 1

// holds the update flags defined above
volatile uint8_t bUpdateFlagsShared;

unsigned long samplerate = 5000;  // 5 second dust sample rate
unsigned long sampletime;         // variable to monitor time of each sample start
int SampleCount;                  // variable to define which number sample in series
int NoOfSamples = 24;             //maximum 24
static long  PM2_Value;
String AQIColour;


// shared variables are updated by the ISR and read by loop.
// In loop we immediatley take local copies so that the ISR can keep ownership of the
// shared ones. To access these in loop
// we first turn interrupts off with noInterrupts
// we take a copy to use in loop and the turn interrupts back on
// as quickly as possible, this ensures that we are always able to receive new signals
volatile uint16_t unPM2_InShared;


// These are used to record the fallng edge of a pulse in the calcInput functions
// They do not need to be volatile as they are only used in the ISR. If we wanted
// to refer to these in loop and the ISR then they would need to be declared volatile
uint32_t ulPM2_Start;




