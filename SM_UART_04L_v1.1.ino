
#define fwVersion "SM_UART_04L_v1.1"
int sampleRate = 2000; //sample rate in mS


#define sensorBaud 9600
#include <U8g2lib.h>   //load U8g2 graphic library https://github.com/olikraus/U8g2_Arduino 

//following sets up SPI OLED 128x64 resolution as Amphenol-AS evaluation kit
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 12, /* data=*/ 11, /* cs=*/ 8, /* dc=*/ 9, /* reset=*/ 10);

#include "SoftwareSerial.h"   //arduino software serial
#define sleepPin 4 //D4, Connector Pin 3, 04L Pin 10 Sleep
#define Rx_Pin  5  //D5, Connector Pin 4, 04L Pin 9 Tx
#define Tx_Pin  6  //D6, Connector Pin 5, 04L Pin 7 Rx
SoftwareSerial mySerial(Rx_Pin, Tx_Pin); // RX, TX               // Declare serial

byte data[32] ; //create an array to store the response
unsigned long sampleTime;         // variable to monitor time of each sample start


//________________________________setup loop, runs once______________________________
void setup()
{
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH); //puts unit active (HIGH), sleep (LOW)

  u8g2.begin();
  Serial.begin(19200); //Opens the main serial port over USB
  mySerial.begin(sensorBaud);  //baud rate, configuration
  //opens sensor serial with baud rate, configuration, pins are preset

  Serial.println(F("Amphenol Advanced Sensors"));
  Serial.println(F(fwVersion));

  u8g2_prepare();
  u8g2.drawStr( 0, 20, fwVersion);
  u8g2.sendBuffer();
  delay(1500);

  for (int i = 128; i > -120; i--) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_helvB14_tr);
    u8g2.drawStr( i, 15, "Amphenol");
    u8g2.setFont(u8g2_font_helvB08_tf);
    u8g2.drawStr( i, 35, "Advanced Sensors");
    u8g2.sendBuffer();
    delay(5);
  }


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
      //Serial.print(( data[10] & 0x3F ) << 8 | data[11]); Serial.print(", "); //PM1 Environment
      //Serial.print(( data[12] & 0x3F ) << 8 | data[13]); Serial.print(", "); //PM2.5 Environment
      //Serial.print(( data[14] & 0x3F ) << 8 | data[15]); Serial.print(", "); //PM10 Environment
      binPrint(data[29]); // Alarm Byte
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



void binPrint(byte printValue)  //prints byte as binary value putting in leading 0s
{
  if (printValue < 2)  {
    Serial.print("0000000");
  }
  else if (printValue < 4)  {
    Serial.print("000000");
  }
  else if (printValue < 8)  {
    Serial.print("00000");
  }
  else if (printValue < 16) {
    Serial.print("0000");
  }
  else if (printValue < 32) {
    Serial.print("000");
  }
  else if (printValue < 64) {
    Serial.print("00");
  }
  else if (printValue < 128) {
    Serial.print("0");
  }

  Serial.print(printValue, BIN);
  Serial.print("  ");
}

//_________________________________________Display Routines____________________
void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_helvB08_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
}

void displayReading() {
  u8g2_prepare();
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
  u8g2.drawStr( 10, 0, "Amphenol Sensors");
  u8g2.drawStr( 20, 55, "SM-UART-04L");
  u8g2.setCursor(80, 15); u8g2.write(181); u8g2.print("g/m"); u8g2.write(179); //display µg/m³

  u8g2.drawStr( 3, 12, "PM1");
  u8g2.drawStr( 3, 22, "PM2.5");
  u8g2.drawStr( 3, 32, "PM10");
  u8g2.drawStr( 3, 42, "Alarm Byte");

  u8g2.setCursor(60, 12); u8g2.print(( data[4] & 0x3F ) << 8 | data[5]);  //PM1 Standard Smoke
  u8g2.setCursor(60, 22); u8g2.print(( data[6] & 0x3F ) << 8 | data[7]);  //PM2.5 Standard Smoke
  u8g2.setCursor(60, 32); u8g2.print(( data[8] & 0x3F ) << 8 | data[9]);  //PM10 Standard Smoke

  u8g2.setCursor(70, 42);
  if (data[29] < 2)  {  //put in leading 0 values
    u8g2.print("0000000");
  }
  else if (data[29] < 4)  {
    u8g2.print("000000");
  }
  else if (data[29] < 8)  {
    u8g2.print("00000");
  }
  else if (data[29] < 16) {
    u8g2.print("0000");
  }
  else if (data[29] < 32) {
    u8g2.print("000");
  }
  else if (data[29] < 64) {
    u8g2.print("00");
  }
  else if (data[29] < 128) {
    u8g2.print("0");
  }

  u8g2.print(data[29], BIN); // Alarm Byte

  u8g2.sendBuffer();
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
