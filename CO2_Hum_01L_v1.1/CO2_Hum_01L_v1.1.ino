
/* This sketch monitors Telaire SM-UART-01L Dust Sensor, T9602-3-D Humidiuty & Tem Sensor, and T6700 
 *  series Carbon Dioxide Sensor. The Sketch will automatically note if one sensor is missing and
 *  not display or send to serial the unavialble sensor values.
 *  
 *  Suggest changes are not made below the first line. 
 *    
 *  the display rate defines how quickly the display changes output, or if in serial mode how often 
 *  values are sent to the serial port. default seting is 5 seconds (5,000 mS)
 *  
 *  Default Serial Communications is set to 9600 baud.
 *  
 *  Sketch cannot display and send to serial simutaneously, so make the unused definition a remark by
 *  using '//'.
*/
//#define USB  //enable or disable monitoring on serial output, alternate with display
#define OELCD_OP    //enable or disable LCD Output, alternate with serial output

unsigned int displayrate = 5000; //delay between display or printed values in mS

/*
Copyright (c) 2016 Amphenol Advanced Sensors
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//__________________________________________________________________________________
#include "Wire.h"
#include <SoftwareSerial.h>

//  Width Guide      "---------------------"
#define SplashScreen "Telaire eval, v1.1"
char ScreenHeader[] = "  Amphenol Sensors";
char ScreenFooter[] = " Telaire Technology";

#ifdef OELCD_OP
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "OELCD.h"
#endif

#include "T9602.h"
#include "T6700.h"
#include "SM-UART-01L.h"

byte error;
byte Connected = 0; // bit values define sensor presence B1 -  T6700, B10 T9602 Temp, B100 T9602 Hum, B1000 Dust
byte data[6];
byte kk = 1;
unsigned int Offset = 0;

//SoftwareSerial mySerial(2,3);
SoftwareSerial mySerial(_DBG_RXPIN_, _DBG_TXPIN_);

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
  display.setCursor(15, 57);
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

boolean readPM()
{
  mySerial.write(READ_VAL_CMD, 8);
  delay(10);
  //while ( !mySerial.available() ) {}
  return true;

}

//reads the dust value from the 01L
void ReadDustValues()
{
  if ( readPM() )
  {
    mySerial.readBytes(buffer3, 14);
    crc = CRC16(buffer3, 12);

    if (crc == (buffer3[12] << 8 | buffer3[13] ))
    {
      //PM1_Value = (buffer3[6] * 255 + buffer3[7]) ;     //PM1
      PM2_Value = (buffer3[8] * 255 + buffer3[9]) ;     //PM22.5
      //PM10_Value = (buffer3[10] * 255 + buffer3[11]) ;   //PM10
      if ( data[1] > MAX_RANGE ) {
        data[1] = MAX_RANGE;
      }
    }


    // Colour Values based on US EPA Air Quality Index for PM 2.5 and PM 10
    if (PM2_Value <= 12 )
    {
      AQIColour = "Green ";
    }
    else if (PM2_Value <= 35 )
    {
      AQIColour = "Yellow";
    }
    else if (PM2_Value <= 55 )
    {
      AQIColour = "Orange";
    }
    else if (PM2_Value <= 150 )
    {
      AQIColour = " Red  ";
    }
    else if (PM2_Value <= 250 )
    {
      AQIColour = "Purple";
    }
    else {
      AQIColour = "Maroon";
    }
  }
}

uint16_t CRC16 (uint8_t *puchMsg, uint8_t u8_Length ) /* The function returns the CRC as a unsigned short type */
{
  uint16_t u16_Reg_CRC = 0xFFFF;
  uint8_t i, j = 0;
  uint8_t u8_Temp_Reg = 0;
  for (i = 0; i < u8_Length; i++)
  {
    u16_Reg_CRC ^= *puchMsg++;
    for (j = 0; j < 8; j++)
    {
      if (u16_Reg_CRC & 0x0001)
      {
        u16_Reg_CRC = u16_Reg_CRC >> 1 ^ 0xA001;
      }
      else
      {
        u16_Reg_CRC >>= 1;
      }
    }
  }
  u8_Temp_Reg = u16_Reg_CRC >> 8;
  return (u16_Reg_CRC << 8 | u8_Temp_Reg );
}

//_______________________________________________setup_________________________________

void setup()
{
#ifdef USB //output to PC through USB
  Serial.begin(9600);     // start serial for output
  Serial.println(SplashScreen);
#endif
  Wire.begin();
  mySerial.begin(SENSOR_BAUD_RATE);     // opens sensor serial port
  mySerial.write(READ_VAL_CMD, 8);
  delay(10);

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

  // test for dust sensor
  if (mySerial.available()) {
    #ifdef USB //output to PC through USB
    Serial.println("Dust Sensor Detected");
    #endif
    Connected = Connected + 8;
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
    Serial.print("PM2 Value"); Serial.print (",");
    Serial.print("PM10 Value"); Serial.print (",");
    Serial.print ("AQIColour");
  }
  Serial.println (" ");

    if ((Connected & B00000001) == 1 ) {    //is T6700 present?
    Serial.print("(ppm)");
    Serial.print (",");
  }
  if ((Connected & B00000110) == 6 ) {    //is T9602 present?
    Serial.print ("(");Serial.write(176); Serial.print("C),"); Serial.print("(%rH),");
  }
  if ((Connected & B00001000) == 8 ) {    //is dust sensor present?
    Serial.print ("("); Serial.write(181); Serial.print("g/m3)"); Serial.print (",(");
    Serial.write(181); Serial.print("g/m3)"); Serial.print(",(AQI Colour Code)"); 
    Serial.print (AQIColour);
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

  if ((Connected & B00000001) == 1 ) {
    getT6700data();
  }
  if ((Connected & B00000110) == 6 ) {
    getT9602data();
  }
  if ((Connected & B00001000) == 8 ) {
    ReadDustValues();
  }


  String SensorReading;
  String AuxInfo;
  String Units;

#ifdef OELCD_OP
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
      AuxInfo = "Humidity";
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

      AuxInfo = "AQI"; AuxInfo += char(32); AuxInfo += "Colour [";
      AuxInfo += String(PM2_Value); AuxInfo += "]";
      Offset = 0;
    }
    else {
      kk++;
    }
  }
if (AuxInfo != ""){
  displayReading(SensorReading, AuxInfo, Units, Offset);
  delay(displayrate);
}
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
    Serial.print(PM10_Value); Serial.print (",");
    Serial.print (AQIColour);
  }
  
  Serial.println ();
  delay(displayrate);
#endif

} //-----------------end of loop-----------------



