unsigned int displayrate = 1000; //delay between display or printed values in mS

//__________________________________________________________________________________

unsigned long sampletime;       // variable for timer

#include "Wire.h"

unsigned char ide_workaround = 0;

#include <SoftwareSerial.h>

#define MAX_RANGE 3000


#define ADDR_6700  0x15 // default I2C slave address

int CO2ppmValue;



//set up LCD
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_MOSI   11 //SDA
#define OLED_CLK   12 //SCL
#define OLED_DC    9 // D/C
#define OLED_CS     11// not used
#define OLED_RESET 10 //RESET
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

SoftwareSerial mySerial(6, 5);

byte buffer3[32];
static unsigned int data[3] = {0};
byte MSB, LSB;


void setup() { //______________________________________SETUP LOOP___________________________
  Serial.begin(19200);      //opens serial port
  Wire.begin();

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("AASv1.5_01L_T67xx");
  display.display();
  delay(2000);

  Serial.println("Amphenol Advanced Sensors AASv1.5_01L T67xx");
  Serial.print("PM2.5 "); Serial.print("µg/m3,"); Serial.println(" CO2 ppm");
  mySerial.begin(9600);     // opens sensor serial port, sets data rate to 9600 bps
}

void loop() {  //___________________________________LOOP_________________________________
  if (millis() >= (displayrate + sampletime))
  {
    sampletime = millis();  //resets timer
    int i;
    static int j = 0;
    uint16_t crc = 0;


    if ( readPM() )
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

    GetCO2PPM();
    /*
      Serial.print(data[0]);
      Serial.print(", ");
      Serial.print(data[1]);
      Serial.print(", ");  */
    Serial.print(data[1]);
    Serial.print(", ");
    Serial.println(CO2ppmValue);


    displayReading();

  }
}
//___________________________________________SUB ROUTINES__________________________________



void displayReading() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("  Amphenol Sensors");
  display.setCursor(12, 57);

  display.setCursor(12, 12);
  display.print("PM2.5        CO2");

  display.setTextSize(1);

  display.setCursor(15, 46);
  display.setTextSize(1);
  display.print("ug/m3       ppm");
  display.setCursor(14, 57);
  display.print("Laser Dust  & CO2");

  display.setTextSize(2);
  display.setCursor(15, 25);
  display.print(data[1]);

  display.setCursor(75, 25);
  display.print(CO2ppmValue);

  display.display();
}


/*void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
  {
  asm volatile ("  jmp 0");
  }*/

boolean readPM()
{

  byte READ_VAL_CMD[8] = {0x20, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x17, 0x7A};

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



void GetCO2PPM()
{
  byte CO2data[6];
  // start I2C
  Wire.beginTransmission(ADDR_6700);
  Wire.write(0x04); Wire.write(0x13); Wire.write(0x8B); Wire.write(0x00); Wire.write(0x01);
  // end transmission
  Wire.endTransmission();
  // read report of current gas measurement in ppm
  Wire.requestFrom(ADDR_6700, 4);    // request 4 bytes from slave device
  CO2data[0] = Wire.read();
  CO2data[1] = Wire.read();
  CO2data[2] = Wire.read();
  CO2data[3] = Wire.read();
  //  Serial.print("MSB: ");  Serial.print(CO2data[2]); Serial.print("  ");
  //  Serial.print("LSB: ");  Serial.print(CO2data[3]); Serial.print("  ");
  CO2ppmValue = ((CO2data[2] & 0x3F ) << 8) | CO2data[3];
  if (CO2ppmValue = 16383) {
    CO2ppmValue = 0;
  }
}
