#define version "T6700-R15 I2C v1.0"
#define readDelay 5  //delay between I2C write & read requests in mS (10 recommended)
#define measureDelay 1000  //delay between measure and read PPm requests in mS (2250 min recommended)
#define ADDR_6700  0x15 // default I2C slave address is 0x15
#define sampleTime 30000 //time between measures

#include <Wire.h>   //I2C 2 wire library from rheingoldheavy.com/changing-the-i2c-library/
#include <SPI.h>   //comms library for OLED
#include <U8g2lib.h>  //library for display
#include <Arduino.h>

//setup screen
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 12, /* data=*/ 11, /* cs=*/ 8, /* dc=*/ 9, /* reset=*/ 10);

unsigned long outputTimer;
int CO2ppmValue;

void setup()
{
  byte error;

  pinMode(2, INPUT);   //define pin 2 for switch

  Serial.begin(19200); //start USB Serial port
  Wire.begin();         //start I2C bus
  u8g2.begin();        //start OLED Driver

  Serial.println(F(version));

  u8g2_prepare();
  u8g2.drawStr( 5, 30, version);
  u8g2.sendBuffer();
  delay(1500);

  u8g2_prepare();
  for (int i = 128; i > -120; i--) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_helvB14_tr);
    u8g2.drawStr( i, 15, "Amphenol");
    u8g2.setFont(u8g2_font_helvB08_tr);
    u8g2.drawStr( i, 35, "Advanced Sensors");
    u8g2.sendBuffer();
    delay(10);
  }

  Serial.print (F("T67xx Serial Number: "));
  Serial.println (GetSerialNo());
  Serial.print (F("Firmware Number: "));
  Serial.println (GetFirmware());

  Serial.print(F("Sensor Status "));
  if (Status() == true)        //response & is status OK?
  {
    Serial.println(F("OK"));
  }
  else
  {
    Serial.println(F("NOK"));
  }

  Serial.print(F("Mode is: "));
  if ( readMoD() == true ) {
    Serial.println(F("Measure on Demand"));
  }
  else
  {
    Serial.println(F("R15"));
  }

}

void loop() {
  bool pushButton = false;
  unsigned long lastDebounceTime = millis();
  while ( digitalRead(2) == HIGH ) {
    if (millis() > 100 + lastDebounceTime) {             //number defines debounce time
      // whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:

      pushButton = true;
    }
  }
  if (pushButton == true ) Serial.print("button initiated ");
  // test if timer has expired or button is pressed
  if (millis() > sampleTime + outputTimer || pushButton == true ) {
    CO2ppmValue = GetCO2PPM();
    Serial.println(CO2ppmValue);
    outputTimer = millis();
  }
  displayReading(CO2ppmValue, int((millis() - outputTimer ) / 1000));

}


//___________________________sub routines hereafter__________

//______________________________________T6700 routines_______

unsigned long GetSerialNo()
{
  byte data[6];
  Wire.beginTransmission(ADDR_6700);
  Wire.write(0x03); Wire.write(0x0F); Wire.write(0xA2); Wire.write(0x00); Wire.write(0x02);
  // end transmission
  Wire.endTransmission();
  delay(readDelay);
  Wire.requestFrom(ADDR_6700, 6);    // request 6 bytes from slave device
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  data[3] = Wire.read();
  data[4] = Wire.read();
  data[5] = Wire.read();

  return ((unsigned long)data[2] << 24) | ((unsigned long)data[3] << 16) | ((unsigned long)data[4] << 8) | (unsigned long)data[5];
}

unsigned int  GetFirmware()
{
  byte data[4];
  Wire.beginTransmission(ADDR_6700);
  Wire.write(0x04); Wire.write(0x13); Wire.write(0x89); Wire.write(0x00); Wire.write(0x01);
  // end transmission
  Wire.endTransmission();
  delay(readDelay);
  Wire.requestFrom(ADDR_6700, 4);    // request 4 bytes from slave device
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();    //MSB
  data[3] = Wire.read();    //LSB
  return ((unsigned int)data[2] << 8) | ((unsigned int)data[3]);
}

bool Status() {
  byte data[4];
  Wire.beginTransmission(ADDR_6700);
  Wire.write(0x04); Wire.write(0x13); Wire.write(0x8A); Wire.write(0x00); Wire.write(0x01);
  // end transmission
  Wire.endTransmission();
  delay(readDelay);
  Wire.requestFrom(ADDR_6700, 4);    // request 4 bytes from slave device
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();    //MSB
  data[3] = Wire.read();    //LSB

  if (data[0] == 0x04 && data[3] == 0x00)        //response & is status OK?
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool readMoD() {
  byte data[5];
  Wire.beginTransmission(ADDR_6700);
  Wire.write(0x05); Wire.write(0x03); Wire.write(0xED); Wire.write(0xFF); Wire.write(0x00);
  // end transmission
  Wire.endTransmission();
  delay(readDelay);
  Wire.requestFrom(ADDR_6700, 5);    // request 5 bytes from slave device
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  data[3] = Wire.read();
  data[4] = Wire.read();

  if (data[0] == 0x05 && data[4] == 0x00)        //response & is status OK?
  {
    return true;
  }
  else
  {
    return false;
  }
}


int GetCO2PPM()
{
  byte data[5];

  Wire.beginTransmission(ADDR_6700);
  Wire.write(0x05); Wire.write(0x03); Wire.write(0xF3); Wire.write(0x00); Wire.write(0x00);
  // end transmission
  Wire.endTransmission();
  delay(readDelay);
  Wire.requestFrom(ADDR_6700, 5);    // request 5 bytes from slave device
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  data[3] = Wire.read();
  data[4] = Wire.read();

  if (data[1] == 0x03 && data[4] == 0x00 ) {  //test to ensure sensor responded to measure command
    delay(measureDelay);

    Wire.beginTransmission(ADDR_6700);
    Wire.write(0x04); Wire.write(0x13); Wire.write(0x8B); Wire.write(0x00); Wire.write(0x01);
    // end transmission
    Wire.endTransmission();
    // read report of current gas measurement in ppm
    delay(readDelay);   //delay in mS
    Wire.requestFrom(ADDR_6700, 4);    // request 4 bytes from slave device
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();

    return ((data[2] & 0x3F ) << 8) | data[3];

  } else {
    return ((0xC3 & 0x3F ) << 8) | 0x50;   //or whatever value for failure you want
  }


}

//____________________________display routines___________________________________
void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
}

void displayReading(int aa, int bb) {
  u8g2_prepare();

  u8g2.drawStr(13, 0, "Amphenol Sensors");
  u8g2.drawStr(0, 50, "Last Measure");
  u8g2.setCursor(74, 50);
  u8g2.print(bb);
  u8g2.drawStr(89, 50, "sec ago");
  u8g2.drawStr(0, 25, "CO2");
  u8g2.drawStr(102, 25, "ppm");

  u8g2.setFont(u8g2_font_helvB14_tr);
  u8g2.setCursor(40, 20);
  u8g2.print(aa);

  u8g2.sendBuffer();
}
