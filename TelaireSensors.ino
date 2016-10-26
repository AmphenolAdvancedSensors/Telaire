//#define USB  //enable or disable monitoring on serial output, alternate with display, 115200 baud
#define OELCD_OP    //enable or disable LCD Output, alternate with serial output

#define DustSensor true  // true or false dependent upon if dust sensor present, true by default as we cannot handshake

#include "Wire.h"
#include <SPI.h>
#include <PinChangeInterrupt.h>

//  Width Guide      "---------------------"
#define SplashScreen "Telaire eval, v1.4"
char ScreenHeader[] = "  Amphenol Sensors";
char ScreenFooter[] = " Telaire Technology";

#ifdef OELCD_OP
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "OELCD.h"
#endif

#include "T9602.h"
#include "T6700.h"
#include "SM-PWM-01C.h"

byte error;
byte Connected = 0; // bit values define sensor presence B1 -  T6700, B10 T9602 Temp, B100 T9602 Hum, B1000 Dust
byte data[6];
byte kk = 1;
unsigned int Offset = 0;

//_______________________________________________setup_________________________________

void setup()
{
#ifdef USB //output to PC through USB
  Serial.begin(115200);  // start serial for output
  Serial.println(SplashScreen);
#endif

  Wire.begin();

#ifdef OELCD_OP
  // by default, we'll generate the high voltage from the 3.3v line internally
  display.begin(SSD1306_SWITCHCAPVCC);
  displaySetupScreen();
#endif

  Wire.beginTransmission(ADDR_6700); // check if T6700 series is present
  error = Wire.endTransmission();
  if (error == 0)
  {
#ifdef USB //output to PC through USB
    Serial.println("T6700 Detected");
#endif
    Connected = 1;
  }

  Wire.beginTransmission(ADDR_9602); // check if T9602 series is present
  error = Wire.endTransmission();
  if (error == 0)
  {
#ifdef USB //output to PC through USB
    Serial.println("T9602 Detected");
#endif
    Connected = Connected + 6;
  }

  // assume Dust Sensor is present as we cannot handshake it
  if (DustSensor) {
#ifdef USB //output to PC through USB
    Serial.println("Dust Sensor Detected");
#endif
    Connected = Connected + 8;

    pinMode(PM2_IN_PIN, INPUT);  //setup SWM PWM 01C Pins to work with pininterrupt algorithm
    pinMode(PM2_OUT_PIN, OUTPUT);

    // using the PinChangeInt library, attach the interrupts
    // used to read the channels
    attachPCINT(digitalPinToPCINT(PM2_IN_PIN), calcPM2, CHANGE);
  }

#ifdef USB //output to PC through USB
  if ((Connected & B00000001) == 1 ) {
    Serial.print("CO2ppm");
    Serial.print (",");
  }
  if ((Connected & B00000110) == 6 ) {
    Serial.print("Temperature"); Serial.print (",");
    Serial.print("Humidity"); Serial.print (",");
  }
  if ((Connected & B00001000) == 8 ) {
    Serial.print("PM2_Value"); Serial.print (",");
    Serial.print ("AQIColour");
  }
  Serial.println (" ");

#endif

#ifdef OELCD_OP
  if ((Connected & B00000001) == 1 ) {
    displayReading("", "CO2 Sensor", "", Offset);
    delay(1000);
  }
  if ((Connected & B00000110) == 6 ) {
    displayReading("", "T9602 Sensor", "", Offset);
    delay(1000);
  }
  if ((Connected & B00001000) == 8 ) {
    displayReading("", "Dust Sensor", "", Offset);
    delay(1000);
  }
#endif
}

//___________________________________________________loop______________________

void loop() {
  if (millis() >= (samplerate + sampletime))
  {
    if ((Connected & B00000001) == 1 ) {
      getT6700data();
    }
    if ((Connected & B00000110) == 6 ) {
      getT9602data();
    }
    if ((Connected & B00001000) == 8 ) {
      CalculateDustValue();
    }
    sampletime = millis();  //resets timer before printing output

#ifdef OELCD_OP
    String SensorReading;
    String AuxInfo;
    String Units;

    if (kk >= 5) {
      kk = 1;
    }


    if (kk == 1) {
      if ((Connected & B00000010) == 2 )    //is T9602 temp present?
      { SensorReading = String(temperature);
        SensorReading.remove(4);
        Units =  char(247); Units += "C";
        AuxInfo = "Temperature";
        Offset = 10;
      }
      else {
        kk++;
      }
    }

    if (kk == 2) {
      if ((Connected & B00000100) == 4 ) {    //is T9602 hum present?
        SensorReading = String(humidity);
        SensorReading.remove(4);
        Units = "%rH";
        AuxInfo = char(32); AuxInfo += char(32);
        AuxInfo += "Humidity";
        Offset = 10;
      }
      else {
        kk++;
      }
    }

    if (kk == 3) {
      if ((Connected & B00000001) == 1 ) //is T6700 present?
      { SensorReading = CO2ppmValue;
        SensorReading.remove(4);
        Units = "ppm";
        AuxInfo = "Carbon"; AuxInfo += char(32); AuxInfo += "Dioxide";
        Offset = 15;
      }
      else {
        kk++;
      }
    }


    if (kk == 4) {
      if ((Connected & B00001000) == 8) {    //is dust sensor present?
        SensorReading = String(AQIColour);
        Units = "";
        
        AuxInfo = "AQI"; AuxInfo += char(32); AuxInfo += "Colour  [";
        AuxInfo += String(PM2_Value); AuxInfo += "]";
        Offset = 0;
      }
      else {
        kk++;
      }
    }

    displayReading(SensorReading, AuxInfo, Units, Offset);

    kk++;


#endif

#ifdef USB //output to PC through USB
    if ((Connected & B00000001) == 1 ) {    //is T6700 present?
      Serial.print(CO2ppmValue);
      Serial.print (",");
    }
    if ((Connected & B00000110) == 6 ) {    //is T9602 present?
      Serial.print(temperature); Serial.print (",");
      Serial.print(humidity); Serial.print (",");
    }
    if ((Connected & B00001000) == 8 ) {    //is dust sensor present?
      Serial.print(PM2_Value); Serial.print (",");
      Serial.print (AQIColour);
    }
    Serial.println ("");
#endif
  }
}

//________________________________Screen Sub Routines_______________________________


#ifdef OELCD_OP
void displaySetupScreen() {
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 30);
  display.print(SplashScreen);
  display.display();
  delay(2000);
  display.clearDisplay();
}

void displayReading(String aa, String bb, String cc, int dd) {
  display.clearDisplay();

  // display header detail
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0); //header
  display.print(ScreenHeader);

  // display screen header 2nd line
  display.setCursor(0, 12);
  display.setTextSize(1);
  display.print(ScreenFooter);

  // display reading
  display.setTextSize(3);
  display.setCursor(10 + dd, 28);
  display.print(aa);  //Sensor Reading

  // display units
  display.setTextSize(1);
  display.setCursor(100, 35);
  display.print(cc);  //units

  // display aux info
  display.setTextSize(1);
  display.setCursor(25, 57);
  display.print(bb);  //aux info



  display.display();
}
#endif

//________________________________________Sensor Sub Routines____________________________

void getT9602data() //gets temp and hum values from T9602 over I2C
{
  Wire.beginTransmission(ADDR_9602);
  Wire.write(0);
  Wire.endTransmission();

  byte aa, bb, cc, dd;

  Wire.requestFrom(ADDR_9602, 4);
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  data[3] = Wire.read();


  // humidity = (rH_High [5:0] x 256 + rH_Low [7:0]) / 16384 x 100
  humidity = (float)(((data[0] & 0x3F ) << 8) + data[1]) / 16384.0 * 100.0;
  humidity = round(humidity);
  // temperature = (Temp_High [7:0] x 64 + Temp_Low [7:2]/4 ) / 16384 x 165 - 40
  temperature = (float)((unsigned)(data[2]  * 64) + (unsigned)(data[3] >> 2 )) / 16384.0 * 165.0 - 40.0;
  temperature = round(temperature * 10);
  temperature = temperature / 10;

}


//gets CO2 ppm value from T6700 series over I2C bus
void getT6700data()
{
  // start I2C
  Wire.beginTransmission(ADDR_6700);
  Wire.write(0x04); Wire.write(0x13); Wire.write(0x8B); Wire.write(0x00); Wire.write(0x01);
  // end transmission
  Wire.endTransmission();

  // read report of current gas measurement in ppm
  delay(1);
  Wire.requestFrom(ADDR_6700, 6);    // request 4 bytes from slave device
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  data[3] = Wire.read();
  //  Serial.print("Func code: "); Serial.println(func_code);
  //  Serial.print("byte count: "); Serial.println(byte_count);
  //  Serial.print("MSB: ");  Serial.print(data[2]); Serial.print("  ");
  //  Serial.print("LSB: ");  Serial.print(data[3]); Serial.print("  ");
  CO2ppmValue = ((data[2] & 0x3F ) << 8) | data[3];
}


//calculates the dust value from the stored
void CalculateDustValue()
// sample times accumulated by the interupts.
{ 
  // create local variables to hold a local copies of the channel inputs
  // these are declared static so that thier values will be retained
  // between calls to loop.
  static uint16_t unPM2_In;
  static uint16_t unPM2_Time;
  // local copy of update flags
  static uint8_t bUpdateFlags;
  static long    PM2_Output[25];

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

  PM2_Output[SampleCount] = unPM2_Time ;
  unPM2_Time = 0;

  PM2_Output[0] = PM2_Output[1] + PM2_Output[2] + PM2_Output[3] + PM2_Output[4] + PM2_Output[5] + PM2_Output[6] + PM2_Output[7] + PM2_Output[8] + PM2_Output[9] + PM2_Output[10] + PM2_Output[11] + PM2_Output[12] + PM2_Output[13] + PM2_Output[14] + PM2_Output[15] + PM2_Output[16] + PM2_Output[17] + PM2_Output[18] + PM2_Output[19] + PM2_Output[20] + PM2_Output[21] + PM2_Output[22] + PM2_Output[23] + PM2_Output[24];

  /*Serial.println();   //use this to see sample data fields being filled in serial mode
  Serial.print (PM2_Output[1]) ; Serial.print("\t"); Serial.print (PM2_Output[2]) ; Serial.print("\t"); Serial.print (PM2_Output[3]) ; Serial.print("\t"); Serial.print (PM2_Output[4]) ; Serial.print("\t"); Serial.print (PM2_Output[5]) ; Serial.print("\t"); Serial.print (PM2_Output[6]) ; Serial.print("\t"); Serial.print (PM2_Output[7]) ; Serial.print("\t"); Serial.print (PM2_Output[8]) ; Serial.print("\t"); Serial.print (PM2_Output[9]) ; Serial.print("\t"); Serial.print (PM2_Output[10]) ; Serial.print("\t"); Serial.print (PM2_Output[11]) ; Serial.print("\t"); Serial.print (PM2_Output[12]) ; Serial.println("\t"); Serial.print (PM2_Output[13]) ; Serial.print("\t"); Serial.print (PM2_Output[14]) ; Serial.print("\t"); Serial.print (PM2_Output[15]) ; Serial.print("\t"); Serial.print (PM2_Output[16]) ; Serial.print("\t"); Serial.print (PM2_Output[17]) ; Serial.print("\t"); Serial.print (PM2_Output[18]) ; Serial.print("\t"); Serial.print (PM2_Output[19]) ; Serial.print("\t"); Serial.print (PM2_Output[20]) ; Serial.print("\t"); Serial.print (PM2_Output[21]) ; Serial.print("\t"); Serial.print (PM2_Output[22]) ; Serial.print("\t"); Serial.print (PM2_Output[23]) ; Serial.print("\t"); Serial.print (PM2_Output[24]);
  Serial.print("\t"); Serial.print (PM2_Output[0]); Serial.println("\t");
  */
  
  /* converts LP outputs to values, calculate % LPO first, then converet to µg/m3 assuming conversion is linear
              output (µS)                           concentration change (250 or 600)
     -----------------------------------    x 100 x ---------------------------------  + offset (0 or 250)
     sample rate (mS) x 1000 x NoOfSamples               percentage change (3 0r 7)

  */
  if (PM2_Output[0] / (samplerate * NoOfSamples * 10 ) >= 3);
  {
    PM2_Value = round((float)PM2_Output[0] / (samplerate * NoOfSamples * 10 ) * 600 / 7 + 250);
  }
  {
    PM2_Value = round((float)PM2_Output[0] / (samplerate * NoOfSamples * 10 ) * 250 / 3);
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
  if (PM2_Value <= 12)
  {
    AQIColour = "Green ";
  }
  else if (PM2_Value <= 35)
  {
    AQIColour = "Yellow";
  }
  else if (PM2_Value <= 55)
  {
    AQIColour = "Orange";
  }
  else if (PM2_Value <= 150)
  {
    AQIColour = char(32);
    AQIColour += "Red";
  }
  else if (PM2_Value <= 250)
  {
    AQIColour = "Purple";
  }
  else {
    AQIColour = "Maroon";
  }
}

// simple interrupt service routine
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


