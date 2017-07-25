/* Connection for T6613, T6615, T6617 UART is all the same. Module can take power from Arduino.
    T66xx   Arduino
     A       Pin 2
     B       Pin 3
     C       +5V
     2       Ground
*/

#include <SoftwareSerial.h>

SoftwareSerial T66xx(8,9); // Sets up a serial port using pin 9 for Rx (A on T66xx)
// and pin 10 for Tx (B on T66xx)

//Define Global Variables
byte readCO2[] = {0xFF, 0XFE, 0X02, 0X02, 0X03}; //Command packet to read Co2 (see app note)
byte readStatus[] = {0xFF, 0XFE, 0X01, 0XB6};    //Command Packet to check Status

byte data[20] ; //create an array to store the response
byte Bytes;     //variable for bytes to request

//setup loop, runs once ------------------------------------------------------------------
void setup()
{
  Serial.begin(9600); //Opens the main serial port over USB
  T66xx.begin(19200); //Opens the T66xx serial port at 19200 baud, 8-N-1 is default

  Serial.println("Amphenol Advanced Sensors");
  Serial.println("CO2 Value from T66xx series over UART");

  for (int i = 0; i < 20; i++)  //10 second delay to allow sensor module to boot
  {
    Serial.print("-");
    delay(500);
  }
  Serial.println();

}


//main loop runs continuously
void loop()
{
  sendRequest(readStatus, 4); //read status

  if (data[3] == 0x00 )        //if status is normal then read ppm value
  {
    sendRequest(readCO2, 5);
    if (data[1] == 0xFA)      //is reading OK?
    {
      short CO2ppmValue = ((data[3] & 0x3F ) << 8) | data[4];
      // Note:T6617 value needs be divided by 100 to give %CO2 by volume e.g. 1235 Output = 12.35% CO2
      
      Serial.print(CO2ppmValue);
      Serial.println(" ppm");
    }

  }
  else
  {
    Serial.println("-");
  }
  delay(5000);
}


//sub routine to read value from module --------------------------------------------
void sendRequest(byte packet[], byte Length)
{


  while (!T66xx.available()) //keep sending request until we start to get a response
  {
    T66xx.write(packet, Length);
    delay(50);
  }
  int timeout = 0; //set a timeout counter
  while (T66xx.available() < Length ) //Wait to get a 7 byte response
  {
    timeout++;
    if (timeout > 10) //if it takes too long there was probably an error
    {
      while (T66xx.available()) //flush whatever we have
        T66xx.read();
      break; //exit and try again
    }
    delay(50);
  }
  for (int i = 0; i < Length; i++)   //read values
  {
    data[i] = T66xx.read();
    //Serial.print(data[i], HEX); Serial.print(" ");
  }
  for (int i = Length; i < 20; i++)  //clear any remaining array values
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

