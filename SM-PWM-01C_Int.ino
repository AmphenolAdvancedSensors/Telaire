#include <PinChangeInterrupt.h>
#include <Wire.h>
//#define CPMonitorOP  //enable or disable monitoring on serial output
#define OELCD_OP

//  Width Guide       "----------------------"
#define SplashScreen "SM-PWM-01C, v1.2"
char ScreenHeader[] = "  Advanced Sensors";
char ScreenFooter[] = "  Evaluation Only";

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
#define PM10_IN_PIN 8   //input for PM10 signal, P2
#define PM2_IN_PIN 7  //input for PM2 signal, P1


// Assign your channel out pins - built in LED is 13
#define PM10_OUT_PIN 3
#define PM2_OUT_PIN 13

// These bit flags are set in bUpdateFlagsShared to indicate which
// channels have new signals
#define PM10_FLAG 1
#define PM2_FLAG 2

// holds the update flags defined above
volatile uint8_t bUpdateFlagsShared;

unsigned long samplerate = 5000;
unsigned long sampletime;
int SampleCount;
int NoOfSamples = 12;  //maximum 14
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

void setup()
{
  SWM_PM_SETUP();
  // using the PinChangeInt library, attach the interrupts
  // used to read the channels
  attachPCINT(digitalPinToPCINT(PM10_IN_PIN), calcPM10, CHANGE);
  attachPCINT(digitalPinToPCINT(PM2_IN_PIN), calcPM2, CHANGE);

#ifdef CPMonitorOP //output to PC through USB
  Serial.begin(115200);
  Serial.println("Arduino Dust & CO2 Value Calculation");
  Serial.print("Dust Sensor sample rate of ");  Serial.print(samplerate / 1000); Serial.print(" sec, with rolling average over "); Serial.print(samplerate / 1000 * NoOfSamples); Serial.println(" sec.");
  Serial.println(SplashScreen);
#endif

#ifdef OELCD_OP
  displaySetupScreen();
#endif

  Wire.begin ();
  TWBR = 152;  // 50 kHz 
}

//note: do not use delay() function in main loop
void loop()
{
  if (millis() >= (samplerate + sampletime))
  {

    CalculateDustValue();

#ifdef CPMonitorOP
    Serial.print("PM2.5: "); Serial.print (PM2_Value); Serial.print(" "); Serial.write(181); Serial.print("g/m3"); Serial.print("\t");
    Serial.print("PM10:  "); Serial.print (PM10_Value); Serial.print(" "); Serial.write(181); Serial.print("g/m3  ");
    Serial.print("AQI Colour Code: "); Serial.println(AQIColour);
#endif

#ifdef OELCD_OP
    displayReading(PM2_Value, PM10_Value, AQIColour);
#endif
  }
}
#define ADDR_6713  0x15 // default I2C slave address for T6713


void SWM_PM_SETUP() {
  pinMode(PM10_IN_PIN, INPUT);
  pinMode(PM2_IN_PIN, INPUT);
  pinMode(PM10_OUT_PIN, OUTPUT);
  pinMode(PM2_OUT_PIN, OUTPUT);
}



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
  static long    PM2_Output[15];
  static long    PM10_Output[15];


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
  
  PM2_Output[0] = PM2_Output[1] + PM2_Output[2] + PM2_Output[3] + PM2_Output[4] + PM2_Output[5] + PM2_Output[6] + PM2_Output[7] + PM2_Output[8]+ PM2_Output[9]+ PM2_Output[10]+ PM2_Output[11]+ PM2_Output[12];
  PM10_Output[0] = PM10_Output[1] + PM10_Output[2] + PM10_Output[3] + PM10_Output[4] + PM10_Output[5] + PM10_Output[6] + PM10_Output[7] + PM10_Output[8] + PM10_Output[9] + PM10_Output[10] + PM10_Output[11] + PM10_Output[12];

//  Serial.print (PM2_Output[0]); Serial.print("\t");
//  Serial.print (PM10_Output[0]); Serial.println("\t");


  /* converts LP outputs to values, calculate % LPO first, then converet to µg/m3 assuming conversion is linear
              output (µS)                           concentration change (250 or 600)
     -----------------------------------    x 100 x ---------------------------------  + offset (0 or 250)
     sample rate (mS) x 1000 x NoOfSamples               percentage change (3 0r 7)

  */
  if (PM2_Output[0] / (samplerate * NoOfSamples * 10 ) >= 3 || PM10_Output[0] / (samplerate * NoOfSamples * 10 ) >= 3);
  {
    PM2_Value = round((float)PM2_Output[0] / (samplerate * NoOfSamples * 10 ) * 600 / 7 + 250);
    PM10_Value = round((float)PM10_Output[0] / (samplerate * NoOfSamples * 10 ) * 600 / 7 + 250);
  }
  {
    PM2_Value = round((float)PM2_Output[0] / (samplerate * NoOfSamples * 10 ) * 250 / 3);
    PM10_Value = round((float)PM10_Output[0] / (samplerate * NoOfSamples * 10 ) * 250 / 3);
  }
  bUpdateFlags = 0;  //reset flags and variables

// Serial.print (PM2_Output[SampleCount]); Serial.print("\t");

  if (SampleCount >= NoOfSamples)
  {
    SampleCount = 1;
//    Serial.print (PM2_Output[0]); Serial.print("\t");Serial.println("\t");
  }
  else
  {
    SampleCount++;
  }

  // Colour Values based on US EPA Air Quality Index for PM 2.5 and PM 10
  if (PM2_Value <= 12 && PM10_Value <= 54)
  {
    AQIColour = "Green ";
  }
  else if (PM2_Value <= 35 && PM10_Value <= 154)
  {
    AQIColour = "Yellow";
  }
  else if (PM2_Value <= 55 && PM10_Value <= 254)
  {
    AQIColour = "Orange";
  }
  else if (PM2_Value <= 150 && PM10_Value <= 354)
  {
    AQIColour = " Red  ";
  }
  else if (PM2_Value <= 250 && PM10_Value <= 424)
  {
    AQIColour = "Purple";
  }
  else {
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
  display.print(SplashScreen);
  display.display();
}


void displayReading(int aa, int bb, String cc) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(ScreenHeader);
  
  display.setTextSize(1);
  display.setCursor(0, 18);
  display.print("PM");
  display.setCursor(0, 26);
  display.print("2.5");
  
  display.setCursor(0, 40);
  display.print("PM");
  display.setCursor(0, 48);
  display.print("10");
  
  display.setCursor(0, 58);
  display.print(ScreenFooter);
  
  display.setTextSize(2);
  display.setCursor(21, 18);
  display.print(aa);
  display.setCursor(21, 40);
  display.print(bb);

  display.setTextSize(2);
  display.setCursor(55, 18);
  display.print(cc);
  
  display.display();
}
#endif




