/*
Use serial window with NL & CR at 19200 baud to answer questions about change request,
the sketch scans the I2C bus, requests which sensor you want to change, and then the value
you want to cahnge to. It then rescans bus for value. It does not check for I2C reserved
address ranges.
*/

int Firmware;

#include "Wire.h"
int readline(int readch, char *buffer, int len)
{
  static int pos = 0;
  int rpos;

  if (readch > 0) {
    switch (readch) {
      case '\n': // Ignore new-lines
        break;
      case '\r': // Return on CR
        rpos = pos;
        pos = 0;  // Reset position index ready for next time
        return rpos;
      default:
        if (pos < len - 1) {
          buffer[pos++] = readch;
          buffer[pos] = 0;
        }
    }
  }
  // No end of line has been found, so return -1.
  return -1;
}

int OldI2CAddress = 0;
int NewI2CAddress = 0;
int i = 0;


byte data[10];

void setup() {

  Serial.begin(19200);         // start serial for output
  while (!Serial);              // Leonardo: wait for serial monitor

  Wire.begin();
  Serial.println("Amphenol Advanced Sensors");
  Serial.println("Arduino I2C Change Address for T67xx Series Sensors");

  Serial.print("Initial ");
  ScanI2CBus();
}

void loop() {
  static char buffer[80];

  if (OldI2CAddress <= 0 & i <= 0)
  {
    Serial.println("Enter Address to Change in Decimal");
    i = 1;
  }

  if (OldI2CAddress <= 0) {
    if (readline(Serial.read(), buffer, 80) > 0) {
      OldI2CAddress = atoi(buffer);
      Serial.print("You entered old: ");
      Serial.print(OldI2CAddress);
      Serial.println(" ");

    }
  }
  if (OldI2CAddress > 0 & NewI2CAddress <= 0 & i == 1) {
    Serial.println("Enter New Address in Decimal");
    i = 2;
  }

  if (OldI2CAddress > 0 & NewI2CAddress <= 0) {

    if (readline(Serial.read(), buffer, 80) > 0) {
      NewI2CAddress = atoi(buffer);
      Serial.print("You entered new: ");
      Serial.print(NewI2CAddress);
      Serial.println(" ");
    }
  }
  if (OldI2CAddress > 0 & NewI2CAddress > 0 & i == 2) {
    Serial.println("Are these values correct?");
    i = 3;
  }
  if (OldI2CAddress > 0 & NewI2CAddress > 0) {
    if (readline(Serial.read(), buffer, 80) > 0) {
      String answer = buffer;
      if (answer == "y" | answer == "Y" | answer == "yes" | answer == "Yes" | answer == "YES") {

        i = 4;
        //Sub routine to enter command mode, change address, and revert to normal mode, no reboot.
        ChangeAddress();
        Serial.println();
        Serial.println("Change Made");
        Serial.println();



        Serial.print("After Reset ");
        ScanI2CBus();
      }
      else
      {
        i = 0;
        OldI2CAddress = 0 ;
        NewI2CAddress = 0;
      }
    }
  }



}


// --------------------Subroutines hereafter--------------------------------

void ChangeAddress() {
  // Read Firmware
  Wire.beginTransmission(OldI2CAddress); Wire.write(0x04); Wire.write(0x13); Wire.write(0x89); Wire.write(0x00); Wire.write(0x01);
  Wire.endTransmission();   // end transmission

  delay(500);

  // data fetch of fimware
  Wire.requestFrom(OldI2CAddress, 6);
  data[0] = Wire.read();  data[1] = Wire.read();
  data[2] = Wire.read();  data[3] = Wire.read();
  data[4] = Wire.read();  data[5] = Wire.read();

  // print byte values
  Firmware = ((data[2] & 0x3F ) << 8) | data[3];

  if (Firmware <= 175) {
  //Serial.print("Read Firmware   ");
  //HEXprint(data[0]); HEXprint(data[1]); HEXprint(data[2]); HEXprint(data[3]); HEXprint(data[4]); HEXprint(data[5]);
  Serial.println();
    Serial.print("Firmware version "); Serial.print(Firmware); Serial.println(" too low to change over I2C, use UART");
    return;
  }

  //Write New Address to Module
  Wire.beginTransmission(OldI2CAddress); Wire.write(0x06); Wire.write(0x0F); Wire.write(0xA5); Wire.write(0x00); Wire.write(NewI2CAddress);
  Wire.endTransmission();   // end transmission
  delay(500);

  //Update EEPROM
  Wire.beginTransmission(OldI2CAddress); Wire.write(0x05); Wire.write(0x03); Wire.write(0xED); Wire.write(0xFF); Wire.write(0x00);
  Wire.endTransmission();   // end transmission
  delay(500);

  // Reset
  Wire.beginTransmission(OldI2CAddress); Wire.write(0x05); Wire.write(0x03); Wire.write(0xE8); Wire.write(0xFF); Wire.write(0x00);
  Wire.endTransmission();   // end transmission

  delay (5000);
}

void ScanI2CBus()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address ");
      Serial.print(address, DEC); Serial.println(" (decimal)");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address ");
      Serial.println(address, DEC);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

}

void HEXprint(byte PrintValue)
{
  Serial.print("0x"); if (PrintValue < 16) {
    Serial.print("0");
  } Serial.print(PrintValue, HEX); Serial.print(" ");
}

/*
Copyright (c) 2017 Amphenol Advanced Sensors
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
