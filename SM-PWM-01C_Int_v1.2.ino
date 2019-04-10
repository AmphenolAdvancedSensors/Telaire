
//#define CPMonitorOP  //enable or disable monitoring on serial output
#define OELCD_OP    //enable or disable monitoring on OLED, alternate with serial
#define fwVersion "SM-PWM-01C, v1.2"

unsigned long samplerate = 5000;//sample rate in mS (5000 = 5 seconds)
int NoOfSamples = 24;  //maximum 24

//------------------------------------------------------------------------------------------------------

#include <PinChangeInterrupt.h>

#ifdef OELCD_OP //set up LCD
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_MOSI   11 //SDA
#define OLED_CLK   12 //SCL
#define OLED_DC    9 // D/C
#define OLED_CS     11// not used
#define OLED_RESET 10 //RESET
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#endif

// Assign your channel in pins
#define PM10_IN_PIN 8   //input for PM2 & 10 signal, P1
#define PM2_IN_PIN 7  //input for PM10 signal, P2, used this way initially signals are sorted aroound line 170


// These bit flags are set in bUpdateFlagsShared to indicate which
// channels have new signals
#define PM10_FLAG 1
#define PM2_FLAG 2

// holds the update flags defined above
volatile uint8_t bUpdateFlagsShared;

unsigned long sampletime;
int SampleCount;
static long  PM2_Value;
static long  PM10_Value;
String AQIColour;

// shared variables are updated by the ISR and read by loop.
// In loop we immediatley take local copies so that the ISR can keep ownership of the
// shared ones. To access these in loop
// we first turn interrupts off with noInterrupts
// we take a copy to use in loop and the turn interrupts back on
// as quickly as possible, this ensures that we are always able to receive new signals
volatile uint16_t unPM10_InShared;
volatile uint16_t unPM2_InShared;


// These are used to record the fallng edge of a pulse in the calcInput functions
// They do not need to be volatile as they are only used in the ISR. If we wanted
// to refer to these in loop and the ISR then they would need to be declared volatile
uint32_t ulPM10_Start;
uint32_t ulPM2_Start;

//______________________________________SetUp_______________________
void setup()
{
  pinMode(PM10_IN_PIN, INPUT);
  pinMode(PM2_IN_PIN, INPUT);
  // using the PinChangeInt library, attach the interrupts
  // used to read the channels
  attachPCINT(digitalPinToPCINT(PM10_IN_PIN), calcPM10, CHANGE);
  attachPCINT(digitalPinToPCINT(PM2_IN_PIN), calcPM2, CHANGE);

#ifdef CPMonitorOP //output to PC through USB
  Serial.begin(19200);
  Serial.println(F("Arduino Dust Comparison"));
  Serial.print(F("Dust Sensor sample rate of "));  Serial.print(samplerate / 1000); Serial.print(F(" sec, with rolling average over ")); Serial.print(samplerate / 1000 * NoOfSamples); Serial.println(F(" sec."));
  Serial.println(fwVersion);
#endif

#ifdef OELCD_OP
  displaySetupScreen();
#endif

}

//_________________________________________________MainLoop___________
//note: do not use delay() function in main loop
void loop()
{
  if (millis() >= (samplerate + sampletime))
  {

    CalculateDustValue();

#ifdef CPMonitorOP
    Serial.print(F("PM2.5: ")); Serial.print (PM2_Value); Serial.print(F(", µg/m3")); Serial.print("\t");
    Serial.print(F("PM10:  ")); Serial.print (PM10_Value); Serial.print(F(", µg/m3  "));
    Serial.print(F("AQI Colour Code: ")); Serial.println(AQIColour);
#endif

#ifdef OELCD_OP
    displayReading(PM2_Value, PM10_Value, AQIColour);
#endif
  }
}


//__________________________________SubRoutines_______________________


void CalculateDustValue() {
  // create local variables to hold a local copies of the channel inputs
  // these are declared static so that thier values will be retained
  // between calls to loop.
  static uint16_t unPM10_In;
  static uint16_t unPM2_In;
  static uint16_t unPM10_Time;
  static uint16_t unPM2_Time;
  // local copy of update flags
  static uint8_t bUpdateFlags;
  static long    PM2_Output[25];
  static long    PM10_Output[25];


  // check shared update flags to see if any channels have a new signal
  if (bUpdateFlagsShared)
  {
    noInterrupts(); // turn interrupts off quickly while we take local copies of the shared variables

    // take a local copy of which channels were updated in case we need to use this in the rest of loop
    bUpdateFlags = bUpdateFlagsShared;

    // in the current code, the shared values are always populated
    // so we could copy them without testing the flags
    // however in the future this could change, so lets
    // only copy when the flags tell us we can.

    if (bUpdateFlags & PM10_FLAG)
    {
      unPM10_In = unPM10_InShared;
      unPM10_Time = (unPM10_Time + unPM10_In);
    }

    if (bUpdateFlags & PM2_FLAG)
    {
      unPM2_In = unPM2_InShared;
      unPM2_Time = (unPM2_Time + unPM2_In);
    }

    // clear shared copy of updated flags as we have already taken the updates
    // we still have a local copy if we need to use it in bUpdateFlags
    bUpdateFlagsShared = 0;

    interrupts(); // we have local copies of the inputs, so now we can turn interrupts back on
    // as soon as interrupts are back on, we can no longer use the shared copies, the interrupt
    // service routines own these and could update them at any time. During the update, the
    // shared copies may contain junk. Luckily we have our local copies to work with :-)
  }

  // do any processing from here onwards
  // only use the local values unPM10_In and unPM2_In, the shared
  // variables unPM10_InShared, unPM2_InShared are always owned by
  // the interrupt routines and should not be used in loop

  sampletime = millis();  //resets timer before printing output
  PM2_Output[SampleCount] = unPM2_Time ;
  PM10_Output[SampleCount] = unPM10_Time ;
  unPM2_Time = 0;
  unPM10_Time = 0;

  PM2_Output[0] = 0;      //reset variables
  PM10_Output[0] = 0;

  //until now P1 and P2 are registered as PM10_Output (PM2.5 and PM10 vlaue, P1) and PM2_Output as (PM10 value, P2)
  //the next lines sort the times to represent PM2.5 and PM10 seperately.
  for (int i = 1; i <= NoOfSamples; i++) {
    if (PM10_Output[i] > PM2_Output[i]) {              //if P1 time is greater then P2 time then:
      PM2_Output[0] = PM2_Output[0] + PM10_Output[i] - PM2_Output[i]; //makes PM2 into just PM2 by subtracting PM10 time (P1-P2)
    } else {
      PM2_Output[0] = PM2_Output[0] + PM10_Output[i]; //as P2 is larger than P1, just add P1
    }
    PM10_Output[0] = PM10_Output[0] + PM10_Output[i];  //Uses P1 values as PM10
  }

  /*  for (int i = 0; i <= NoOfSamples; i++) {
      Serial.print(PM2_Output[i]); Serial.print("\t");
      Serial.print(PM10_Output[i]); Serial.print("|\t");
    }
  */

  /* converts LP outputs to values, calculate % LPO first, then convert to µg/m3 assuming conversion is linear
              output (µS)                           concentration change (250 or 600)
     -----------------------------------    x 100 x ---------------------------------  + offset (0 or 250)
     sample rate (mS) x 1000 x NoOfSamples               percentage change (3 0r 7)

  */

  if (PM2_Output[0] / (samplerate * NoOfSamples * 10 ) >= 3 )
  {
    PM2_Value = round((float)PM2_Output[0] / (samplerate * NoOfSamples * 10 ) * 86 + 250);
  } else {
    PM2_Value = round((float)PM2_Output[0] / (samplerate * NoOfSamples * 10 ) * 83);
  }

  if (PM10_Output[0] / (samplerate * NoOfSamples * 10 ) >= 3)
  {
    PM10_Value = round((float)PM10_Output[0] / (samplerate * NoOfSamples * 10 ) * 86 + 250);
  } else {
    PM10_Value = round((float)PM10_Output[0] / (samplerate * NoOfSamples * 10 ) * 83);
  }

  bUpdateFlags = 0;  //reset flags and variables

  if (SampleCount >= NoOfSamples)
  {
    SampleCount = 1;
  }
  else
  {
    SampleCount++;
  }

  // Colour Values based on US EPA Air Quality Index for PM 2.5 and PM 10
  if (PM2_Value <= 12 && PM10_Value <= 54)
  { AQIColour = "Green ";
  }  else if (PM2_Value <= 35 && PM10_Value <= 154)
  { AQIColour = "Yellow";
  }  else if (PM2_Value <= 55 && PM10_Value <= 254)
  { AQIColour = "Orange";
  }  else if (PM2_Value <= 150 && PM10_Value <= 354)
  { AQIColour = " Red  ";
  }  else if (PM2_Value <= 250 && PM10_Value <= 424)
  { AQIColour = "Purple";
  }  else {
    AQIColour = "Maroon";
  }
}

// simple interrupt service routine
void calcPM10()
{
  // if the pin is low, its a falling edge of the signal pulse, so lets record its value
  if (digitalRead(PM10_IN_PIN) == LOW)
  {
    ulPM10_Start = micros();
  }
  else
  {
    // else it must be a rising edge, so lets get the time and subtract the time of the rising edge
    // this gives use the time between the falling and rising edges i.e. the pulse duration.
    unPM10_InShared = (uint16_t)(micros() - ulPM10_Start);
    // use set the PM10_ flag to indicate that a new PM10_ signal has been received
    bUpdateFlagsShared |= PM10_FLAG;
  }
}

void calcPM2()
{
  if (digitalRead(PM2_IN_PIN) == LOW)
  {
    ulPM2_Start = micros();
  }
  else
  {
    unPM2_InShared = (uint16_t)(micros() - ulPM2_Start);
    bUpdateFlagsShared |= PM2_FLAG;
  }
}

#ifdef OELCD_OP
void displaySetupScreen()
{
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print(F(fwVersion));
  display.display();
}


void displayReading(int aa, int bb, String cc) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(15, 0);
  display.print(F("Amphenol Sensors"));

  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print(F("PM"));
  display.setCursor(0, 23);
  display.print(F("2.5"));

  display.setCursor(0, 37);
  display.print(F("PM"));
  display.setCursor(0, 45);
  display.print(F("10"));

  display.setCursor(17, 55);
  display.print(F("Evaluation Only"));

  display.setCursor(60, 16);
  display.print(F("ug/m3"));

  display.setTextSize(2);
  display.setCursor(21, 15);
  display.print(aa);
  display.setCursor(21, 37);
  display.print(bb);

  display.setTextSize(2);
  display.setCursor(55, 37);
  display.print(cc);

  display.display();
}
#endif
