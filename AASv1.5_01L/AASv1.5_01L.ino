unsigned int displayrate = 1000; //delay between display or printed values in mS

//__________________________________________________________________________________

unsigned long sampletime;       // variable for timer
unsigned char ide_workaround = 0;

#include <SoftwareSerial.h>
#define ENABLE_LCD
#define MAX_RANGE 3000


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


//set up laser sensor
#define SENSOR_BAUD_RATE 9600
#define DEBUG_BARUD_RATE 19200

#define _DBG_RXPIN_ 6
#define _DBG_TXPIN_ 5
SoftwareSerial mySerial(_DBG_RXPIN_, _DBG_TXPIN_);

byte buffer3[32];
static unsigned int data[3] = {0};
byte MSB, LSB;

byte READ_VAL_CMD[8] = {0x20, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x17, 0x7A};

void setup() {
  Serial.begin(19200);
  mySerial.begin(SENSOR_BAUD_RATE);     // opens serial port, sets data rate to 9600 bps

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("AASv1.5_01L");
  display.display();
  delay(2000);
  displayTitle();
  display.display();

  Serial.println(F("Amphenol Advanced Sensors AASv1.5_01L"));
  Serial.print(F("PM2.5 µg/m3"));

}

void loop() {
    if (millis() >= (displayrate + sampletime))
  {
    sampletime = millis();  //resets timer
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
      //data[0] = buffer3[6] * 255 + buffer3[7];  //PM1.0
      data[1] = buffer3[8] * 255 + buffer3[9];  //PM2.5
      //data[2] = buffer3[10] * 255 + buffer3[11]; //PM10
      if ( data[1] > MAX_RANGE ) {
        data[1] = MAX_RANGE;
      }
    }
  }

  Serial.println(data[1]);

  displayReading();

  }
}

void displayTitle()
{
  display.clearDisplay();
  display.setCursor(15, 55);
  display.setTextSize(1);
  display.print("Laser Dust Reading");
  //display.display();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(" Advanced Sensors");

}

void displayReading() {
  //display.clearDisplay();
  displayTitle();
  display.setTextSize(3);
  display.setCursor(5, 16);

  //display.print(data[1]);
  display.setCursor(30, 18);
  display.print(data[1]);

  display.setTextSize(1);
  display.setCursor(83, 19);
  display.print("ug/m3");

  display.display();
}

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

