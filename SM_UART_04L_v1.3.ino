
#define fwVersion "SM_UART_04L_v1.1"
#define sampleRate 2000     //sample rate in ms
#define sensorBaud 9600

// Include libraries for display and communications
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, 11, 12, 9, 10, 11); //(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS)

#include "SoftwareSerial.h"   //arduino software serial
#define sleepPin 4 //D4, Connector Pin 3, 04L Pin 10 Sleep
#define Rx_Pin  6  //D5, Connector Pin 4, 04L Pin 9 Tx
#define Tx_Pin  5  //D6, Connector Pin 5, 04L Pin 7 Rx
SoftwareSerial mySerial(Rx_Pin, Tx_Pin); // RX, TX               // Declare serial

byte data[32] ;                   //create an array to store the response
unsigned long sampleTime;         // variable to monitor time of each sample start
        
//________________________________setup loop, runs once______________________________
void setup()
{
  digitalWrite(sleepPin, HIGH); //puts unit active (HIGH), sleep (LOW)
  pinMode(sleepPin, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC);
  Serial.begin(19200); //Opens the main serial port over USB
  mySerial.begin(sensorBaud);  //baud rate, configuration
  //opens sensor serial with baud rate, configuration, pins are preset

  Serial.println(F("Amphenol Advanced Sensors"));
  Serial.println(F(fwVersion));
  Serial.print(F("Serial Number: "));
  printSerialNumber();
  Serial.println();

  displayPrepare();
  display.setCursor(10, 30);
  display.print(fwVersion);
  display.display();
  delay(1500);

  displayPrepare();
  display.setCursor(15, 40);
  display.print(F("Advanced Sensors"));
  display.setTextSize(2);
  display.setCursor(17, 20);
  display.print(F("Amphenol"));
  display.display();
  delay(1500);

  Serial.println();
  Serial.println(F("PM1 Standard Smoke (µg/m³), PM2.5 Standard Smoke (µg/m³), PM10 Standard Smoke (µg/m³)"));

  sampleTime = millis();
}


//_________________________main loop runs continuously__________________________

void loop()
{
  if (millis() >= (sampleRate + sampleTime))
  {
    sampleTime = millis();  //resets timer before printing output
    //sendRequest(readPM, 5, 32); //comms request to sensor, command string, string length ex CRC, response length in bytes inc CRC
    if (GetSerialData()) {  //request to get data, if CRC OK, then print and display
      Serial.print(( data[4] & 0x3F ) << 8 | data[5]); Serial.print(", "); //PM1 Standard Smoke
      Serial.print(( data[6] & 0x3F ) << 8 | data[7]); Serial.print(", "); //PM2.5 Standard Smoke
      Serial.print(( data[8] & 0x3F ) << 8 | data[9]); Serial.print(", "); //PM10 Standard Smoke
      displayReading();
    } else {
      Serial.print(F("No data"));
    }
    Serial.println();

  }
}


//___________________________sub routine to read value from module_________________


bool GetSerialData() {  //get lots - post sort method
  byte message[64];
  int CRC = 0;
  mySerial.readBytes(message, 64);   //read 64 streamed bytes

  for ( byte i = 0 ; i < 32 ; i++ ) {  //look for ox42, 0x4D sequence and load data from stream
    if ( message[i] == 0x42 && message[i + 1] == 0x4D ) {
      for ( byte j = 0 ; j < 32 ; j++ ) {
        data[j] = message[i];
        i++;
      }
      break;
    }
  }

  if ((data[30] * 256 + data[31]) == getCS(30)) {  //Cyclical Redundancy Check
    return true;
  }
  else {
    return false;
  }
}

unsigned int getCS(byte len)     // Compute the checksum
{
  unsigned int var = 0;
  for (int i = 0; i < len; i++) {
    var += data[i];
  }
  return var;
}

void printSerialNumber() {
  //{0x20, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x53, 0x7A};  message to call serial number
  mySerial.write((byte)0x20);
  mySerial.write((byte)0x0F);
  mySerial.write((byte)0x00);
  mySerial.write((byte)0x00);
  mySerial.write((byte)0x00);
  mySerial.write((byte)0x00);
  mySerial.write((byte)0x53);
  mySerial.write((byte)0x7A);
  mySerial.readBytes(data, 20);

Serial.print(F("Serial Number: "));
  for (int i = 0; i < 20; i++) {
    if (data[i] > 0x2E) Serial.write(data[i]);  //print serial number ASCII
    data[i] = 0x00;                             //delete data
  }
  
}



//_________________________________________Display Routines____________________
void displayPrepare(void) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void displayReading() {
  displayPrepare();
  /*
     Error Byte 0b0ABCDEFG
    A = 1 Laser error
    B = 1 Laser alarm
    C = 1 High temperature alarm
    D = 1 Low temperature alarm
    E = 1 Fan Error
    F = 1 Fan speed compensation start
    G = 1 Fan speed alarm
  */
  display.setCursor(15, 0);  display.print(F("Amphenol Sensors"));
  display.setCursor(13, 55); display.print(F("SM-UART-04L"));
  display.setCursor(80, 15); display.print("ug/m3"); //display µg/m³

  display.setCursor( 3, 12); display.print("PM1");
  display.setCursor( 3, 22); display.print("PM2.5");
  display.setCursor( 3, 32); display.print("PM10");
  display.setCursor( 3, 42); display.print("Alarm Byte");

  display.setCursor(60, 12); display.print(( data[4] & 0x3F ) << 8 | data[5]);  //PM1 Standard Smoke
  display.setCursor(60, 22); display.print(( data[6] & 0x3F ) << 8 | data[7]);  //PM2.5 Standard Smoke
  display.setCursor(60, 32); display.print(( data[8] & 0x3F ) << 8 | data[9]);  //PM10 Standard Smoke

  display.setCursor(70, 42);
  if (data[29] < 2)  {  //put in leading 0 values
    display.print("0000000");
  }
  else if (data[29] < 4)  {
    display.print("000000");
  }
  else if (data[29] < 8)  {
    display.print("00000");
  }
  else if (data[29] < 16) {
    display.print("0000");
  }
  else if (data[29] < 32) {
    display.print("000");
  }
  else if (data[29] < 64) {
    display.print("00");
  }
  else if (data[29] < 128) {
    display.print("0");
  }

  display.print(data[29], BIN); // Alarm Byte

  display.display();
}
/*
  Copyright (c) 2019 Amphenol Advanced Sensors
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
