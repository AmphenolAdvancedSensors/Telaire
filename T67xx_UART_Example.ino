/* Connection for T67xx UART is all the same. Module can take power from Arduino.
    T67xx   Arduino
     A  Tx   Pin 8 Rx
     B  Rx   Pin 9 Tx
     C       +5V
     2       Ground

Sketch uses serial window at 19200 baud with CR & LF enabled.
The sketch runs sensor for 5 minutes prior to requesting a calibration value and 
a single point recalibration routine.
Suggest module is in a stable environment to run this sketch to maximise accuracy,
eg outside at approx 25°C, set at 420ppm.

The sketch requires the AltSoftSerial library to be loaded.
*/

#include "AltSoftSerial.h"

AltSoftSerial mySerial;               // Declare serial

//Define Global Variables including CRC
byte readCO2[] =    {0x15, 0X04, 0X13, 0X8B, 0X00, 0X01, 0X46, 0X70}; //Command packet to read Co2 (see app note)
byte readStatus[] = {0x15, 0X04, 0X13, 0X8A, 0X00, 0X01, 0X17, 0XB0}; //Command Packet to check Status
byte readSerial[] = {0x15, 0X04, 0X13, 0X8A, 0X00, 0X01, 0X17, 0XB0};

byte data[20] ; //create an array to store the response
byte Bytes;     //variable for bytes to request

//________________________________setup loop, runs once______________________________
void setup()
{
  pinMode(13, OUTPUT);
  Serial.begin(19200); //Opens the main serial port over USB
  mySerial.begin(19200, SERIAL_8E1);  //baud rate, configuration

  Serial.println("Amphenol Advanced Sensors");
  Serial.println("CO2 Value from T67xx series over UART");

  for (int i = 0; i < 20; i++)  //15 second delay to allow sensor module to boot
  {
    Serial.print("-");
    delay(500);
  }
  Serial.println();

  sendRequest(readStatus, 8, 7);  //command, write bytes, read bytes
  if (data[4] == 0x00 )        //is status OK?
  {
    Serial.print("Status OK");
  }
  else
  {
    Serial.print("Status NOK");
  }

  Serial.println();
}


//_________________________main loop runs continuously__________________________

void loop()
{
  sendRequest(readCO2, 8, 7);

  if (data[0] == 0x15)
  {
    short CO2ppmValue = ((data[3] & 0x3F ) << 8) | data[4];
    Serial.print(CO2ppmValue);
    Serial.print(" ppm");
  }
  Serial.println();

  delay(5000);
}


//___________________________sub routine to read value from module_________________

void sendRequest(byte packet[], byte writeLength, byte readLength)
{
  mySerial.write(packet, writeLength);
  digitalWrite(13, HIGH);
  delay(30);
  digitalWrite(13, LOW);

  int timeout = 0;                  //set a timeout counter
  while (mySerial.available() < readLength ) //Wait to get a 'Length' byte response
  {
    timeout++;
    if (timeout > 10) //if it takes too long there was probably an error
    {
      while (mySerial.available())  //flush whatever we have
        mySerial.read();
      Serial.print("Send Error ");
      break; //exit and try again
    }
  }

  for (int i = 0; i < readLength; i++)   //read values
  {
    data[i] = mySerial.read();
    //Serial.print(data[i], HEX); Serial.print(" ");
  }
  for (int i = readLength; i < 20; i++)  //clear any remaining array values
  {
    data[i] = 0;
  }
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

