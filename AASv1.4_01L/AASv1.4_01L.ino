unsigned char ide_workaround = 0;

#include <SoftwareSerial.h>
#define ENABLE_LCD
#define MAX_RANGE 3000
//#define DEBUG

// display number
#define PN 4


#ifdef ENABLE_LCD
//set up LCD
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_MOSI   11 //SDA
#define OLED_CLK   12 //SCL
#define OLED_DC    9 // D/C
#define OLED_CS     11// not used
#define OLED_RESET 10 //RESET
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
#endif //ENABLE_LCD

#define _DBG_RXPIN_ 6
#define _DBG_TXPIN_ 5
SoftwareSerial mySerial(_DBG_RXPIN_, _DBG_TXPIN_);
#define DebugSerial  Serial

//set up laser sensor
#define SENSOR_BAUD_RATE 9600
#define DEBUG_BARUD_RATE 19200

byte buffer3[32];
static unsigned int data[3] = {0};
byte MSB, LSB;

byte READ_VAL_CMD[8] = {0x20, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x17, 0x7A};

void setup() {
  mySerial.begin(SENSOR_BAUD_RATE);     // opens serial port, sets data rate to 9600 bps
  DebugSerial.begin(DEBUG_BARUD_RATE);
#ifdef ENABLE_LCD
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("AASv1.4_01L");
  display.display();
  delay(2000);
  displayTitle();
  display.display();
#endif //ENABLE_LCD

Serial.println("Amphenol Advanced Sensors AASv1.4_01L");
Serial.println("Note: PM1.0 & PM10 are informational only");
Serial.print("PM1.0, PM2.5, PM10 "); Serial.write(181); Serial.println("g/m3");

}

void loop() {
  int i;
  static int j = 0;
  uint16_t crc = 0;


  if ( readPM() )
    //DebugSerial.println(mySerial.available());
  {
    mySerial.readBytes(buffer3, 14);
    crc = CRC16(buffer3, 12);

    if (crc == (buffer3[12] << 8 | buffer3[13] ))
    {
      data[0] = buffer3[6] * 255 + buffer3[7];  //PM1.0
      data[1] = buffer3[8] * 255 + buffer3[9];  //PM2.5
      data[2] = buffer3[10] * 255 + buffer3[11]; //PM10
      if ( data[1] > MAX_RANGE ) {
        data[1] = MAX_RANGE;
      }
    }
  }

#ifdef DEBUG
  PrintHex8(buffer3, 14);
  PrintHex16(crc);
#endif //DEBUG
  DebugSerial.print(data[0]);
  DebugSerial.print(",");  
  DebugSerial.print(data[1]);
  DebugSerial.print(",");
  DebugSerial.println(data[2]);
#ifdef ENABLE_LCD
  displayReading();
#endif //ENABLE_LCD 
  delay(1000);//match sampling rate
}

#ifdef ENABLE_LCD
void displayTitle()
{
  display.clearDisplay();
  display.setCursor(15, 55);
  display.setTextSize(1);
  display.print("Laser Dust Reading");
  //display.display();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("   Amphenol Sensors");

}

void displayReading() {
  //display.clearDisplay();
  displayTitle();
  display.setTextSize(1);
  display.setCursor(5, 16);
  display.print("PM 2.5");
  
  display.setTextSize(3);
  display.setCursor(50, 16);
  display.print(data[1]);

  display.setCursor(83, 19);
  display.setTextSize(1);
  display.print("ug/m3");
  
  //display.print("  ");
  //display.println(data[0]);
  /*display.setCursor(15, 32);
  display.print(data[7]-data[10]);
  display.print("  ");
  display.print(data[10]-data[12]);*/
  display.display();
}
#endif //ENABLE_LCD

/*void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
  asm volatile ("  jmp 0");
}*/

boolean readPM()
{
  mySerial.write(READ_VAL_CMD, 8);
  delay(10);
  //while ( !mySerial.available() ) {}
  return true;

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


#ifdef DEBUG
void PrintHex8(uint8_t *data, uint8_t length) // prints 8-bit data in hex with leading zeroes
{
  DebugSerial.print("0x");
  for (int i = 0; i < length; i++) {
    if (data[i] < 0x10) {
      DebugSerial.print("0");
    }
    DebugSerial.print(data[i], HEX);
    DebugSerial.print(" ");
  }
}

void PrintHex16(uint16_t data) // prints 16-bit data in hex with leading zeroes
{
  DebugSerial.print("0x");

  uint8_t MSB = byte(data >> 8);
  uint8_t LSB = byte(data);

  if (MSB < 0x10) {
    DebugSerial.print("0");
  } DebugSerial.print(MSB, HEX); DebugSerial.print(" ");
  if (LSB < 0x10) {
    DebugSerial.print("0");
  } DebugSerial.print(LSB, HEX); DebugSerial.print(" ");

}
#endif //DEBUG
