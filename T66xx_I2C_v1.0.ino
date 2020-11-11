/*
   T6613 Connection

   T6613  Arduino
   1      5V Supply
   3      Ground
   5      SCL
   7      SDA

*/

#include "Wire.h"
#define Version "T66xx I2C v1.0"


#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, 11, 12, 9, 10, 11);

byte data[16];

void setup() {
  Serial.begin(115200);  // start serial for output
  Wire.begin();

  Serial.println(Version);
  display.begin(SSD1306_SWITCHCAPVCC);
  displaySetupScreen();

delay(500);

  byte readSerial[] = {0x08, 0x27}; //16 byte response
  messageRequest(readSerial, 2, 16);  //command, write bytes, read bytes
  Serial.print( "Serial: ");
  for (int i = 0; i < 16; i++) {
    Serial.write(data[i]);
  }

  Serial.println();
}

void loop() {
  byte message[] = {0x08, 0x20};
  messageRequest(message, 2, 2);  //address, message, write length, read length

  int sensorReading = ((data[0] & 0x3F ) << 8) | data[1];
  displayReading(sensorReading);
  Serial.println(sensorReading);

  delay(1000);

}
//________________________________________I2C Sub Routine______________________________

void messageRequest(byte message[], int writeLength, int readLength) {
  Wire.beginTransmission(message[0]);
  for (int i = 1; i < writeLength; i++) {
    Wire.write(message[i]);
  }
  Wire.endTransmission();
  delay(5);
  Wire.requestFrom(message[0], readLength);    // request bytes from slave device
  for (int i = 0; i < readLength; i++) {
    data[i] = Wire.read();
    //Serial.print(data[i], HEX); Serial.print("\t");
  }
}

//______________________________Display Routines__________________________________

void displaySetupScreen() {
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 30);
  display.print(Version);
  display.display();
  delay(2000);
  display.clearDisplay();
}

void displayReading(int aa) {
  display.clearDisplay();

  // display header detail
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20, 0); //header
  display.print("Amphenol Sensors");

  // display screen header 2nd line
  display.setCursor(20, 14);
  display.setTextSize(1);


  // display screen header bottom line
  display.setCursor(40, 55);
  display.setTextSize(1);
  display.print("CO2 Sensor");

  // display reading
  display.setTextSize(3);
  display.setCursor(20, 20);
  display.print(aa);  //Sensor Reading

  // display units
  display.setTextSize(1);
  display.setCursor(100, 35);
  display.print("ppm");  //units

  display.display();
}

/*
Copyright (c) 2020 Amphenol Advanced Sensors
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
