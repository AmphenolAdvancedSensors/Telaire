/*
   NOTE: The VZ-89TE part operates on 3V, so either use a voltage divider or 3V Arduino
   variant when powering and communicating with the part.
*/

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
#include "Wire.h"


#define I2CAddress 0x70
long crc;
int R0Value;
int VOCvalue;
int CO2value;
int ResistorValue;
byte data[10];
int i;
int k = 0;
int lastValue;

//set up LCD


//  Width Guide      "---------------------"
#define SplashScreen "VX89-TE, v1.0"
char ScreenHeader[] = "VX-89TE Reader";
char ScreenFooter[] = " Advanced Sensors";
char DisplayUnit[] = "ppb";

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_MOSI   11 //SDA
#define OLED_CLK   12 //SCL
#define OLED_DC    9 // D/C
#define OLED_CS     11// not used
#define OLED_RESET 10 //RESET
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

//___________________________________display routines_______
void displaySetupScreen()
{
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
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

void displayReading(int aa, char* bb, char* cc, char* dd) {
  display.clearDisplay();

  // display header detail
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(cc);

  // display reading and units
  display.setTextSize(3);
  display.setCursor(15, 20);
  display.print(aa);
  display.setCursor(85, 20);
  display.setTextSize(2);
  display.print(bb);

  // display footer detail
  display.setCursor(10, 57);
  display.setTextSize(1);
  display.print(dd);

  display.display();
}

//__________________________________sub routines__________


void VZ89TEReadData(long request)
{

  /*  CRC processing:
    0x0F + 0x0A + 0x0F + 0x42 + 0x00 + 0x00 = 0x6A
    CRC = 0xFF – 0x6A = 0x95 */
  crc = request;
  crc = (byte)(crc + (crc / 0x100));
  crc = 0xFF - crc;

  // Initiate Comms to device, initiate measure and read bytes of data
  Wire.beginTransmission(I2CAddress);
  Wire.write(request);  Wire.write(0);  Wire.write(0);  Wire.write(0);  Wire.write(0);  Wire.write(crc);
  //PrintHex(request); Serial.print(" "); PrintHex(crc); Serial.println(" ");
  Wire.endTransmission();
  delay(100);
  Wire.requestFrom(I2CAddress, 7);

  for (i = 0; i < 7; i++)
  {
    data[i] = Wire.read();
    //PrintHex(data[i]); Serial.print(" ");
  }

  crc = data[0] + data[1] + data[2] + data[3] + data[4] + data[5];

  crc = (byte)(crc + (crc / 0x100));

  crc = 0xFF - crc;
  if (data[6] = crc) {
    Serial.print("CRC OK    ");
  } else {
    Serial.print("CRC NOK   ");
  }

}

void PrintHex(long value)
{
  Serial.print("0x");
  if (value < 16) {
    Serial.print("0");
  }
  Serial.print(value, HEX);
}


//==================================Set up & Loop==================================

void setup()
{

  //pinMode(13, OUTPUT);
  displaySetupScreen();

  delay(2000);

  Wire.begin();

  Serial.begin(19200);
  while (!Serial);             // Leonardo: wait for serial monitor

  Serial.println("MiCS VT-89");

  VZ89TEReadData(0x0D); //date code & revision
  Serial.print("date code & revision  ");  Serial.print(data[2]); Serial.print("-"); Serial.print(data[1]); Serial.print("-"); Serial.print(data[0]); Serial.print(", ");
  Serial.print("Revision: "); Serial.println(data[3]);
  Serial.print("R0 Calibration Value  ");

  VZ89TEReadData(0x10); //R0 Calibration Value
  R0Value = ((data[1] & 0x3F ) << 8) | data[0];
  Serial.print(R0Value); Serial.println(" kohm");

  Serial.println("\nVX89xE Read Value");
}


void loop() {

  VZ89TEReadData(0x0C); //status
  VOCvalue = (data[0] - 13) * 1000 / 229;
  CO2value = (data[1] - 13) * 1600 / 229 + 400;
  ResistorValue = (static_cast<uint32_t>(data[4]) | (static_cast<uint32_t>(data[3]) << 8) | (static_cast<uint32_t>(data[2]) << 16)) * 10;


  if (VOCvalue < 0 ) {
    Serial.print(lastValue); Serial.print(" ppb, ");
  } else {
    lastValue = VOCvalue;
    Serial.print(VOCvalue); Serial.print(" ppb, ");
  }


  Serial.print(CO2value); Serial.print(" ppm, ");
  Serial.print(ResistorValue); Serial.println(" ohm");

  if (k == 1) {
    displayReading(lastValue, "ppb", "VOC Value", ScreenFooter);
    k = 0;
  }
  else {
    displayReading(CO2value, "ppm", "Carbon Dioxide", ScreenFooter);
    k = 1;
  }

  delay(2000);
}



