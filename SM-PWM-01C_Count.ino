/*
Copyright (c) 2016 Amphenol Advanced Sensors
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
/*
   Calculate LPO (Low Pulse Occupancy) and convert to dust concentration level (ug/m^3), conversion based on experience thus no guarranty for accuracy
   The program is just for demo purpose, please use it with own discretion.

   SPI OLED
*/

// workaround fix for Arduino's buggy preprocessor
unsigned char ide_workaround = 0;

#include <LowPassFilter.h>

static double PARAM_A = -0.0058;
static double PARAM_B = 0.42;
static double PARAM_C = 11.5;
static double PARAM_D = 5;

LPH Filter;
//enable display
#define ENABLE_LCD

#ifdef ENABLE_LCD
//set up LCD
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//set up LCD

#define OLED_MOSI   11 //SDA
#define OLED_CLK   12 //SCL
#define OLED_DC    9 // D/C
#define OLED_CS     11// not used
#define OLED_RESET 10 //RESET
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);


#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#endif // ENABLE_LCD

#include <digitalWriteFast.h>
#define DEBUG_BAUD_RATE 9600 // baud rate for serial output

byte DUST_PWM_PIN = 8;

unsigned long TimeOfsample = 0;

#ifdef ENABLE_LCD
void displayReading(int data, int LPO) {
  display.clearDisplay();
  display.setCursor(2, 55);
  display.setTextSize(1);
  display.print("  IR Dust Tech.");

  display.setTextSize(1);
  display.setCursor(20, 1);
  display.print("Amphenol Count");

  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print("PM");
  display.setCursor(0, 24);
  display.print("2.5");
  
  display.setCursor(0, 37);
  display.print("PM");
  display.setCursor(0, 45);
  display.print("10");
  
  display.setTextSize(2);
  display.setCursor(21, 16);
  display.print(data);
  display.setCursor(21, 37);
  display.print(" - ");

  display.display();
}
#endif //ENABLE_LCD

void printHeader()
{
  Serial.println("Time,P1_1,P2_1");
}

// the setup function runs once when you press reset or power the board
void setup() {
  //Wire.begin();
  Serial.begin(DEBUG_BAUD_RATE);

  pinMode(DUST_PWM_PIN, INPUT);

  printHeader();


#ifdef ENABLE_LCD
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(5, 5);
  display.print("Amphenol");
  display.setCursor(5, 25);
  display.print("Advanced");  display.setCursor(5, 45);
  display.print("Sensors");
  display.display();
  delay(2000);
  display.display();
#endif //ENABLE_LCD
}

// the loop function runs over and over again forever
void loop() {
  double t_x = 0;
  double t_x2 = 0;
#ifdef ENABLE_LCD
  static int lastreading = 0;
#endif //ENABLE_LCD

  static int TimesOfInt = 0;
  static int CntOfLP = {
    0
  };
  unsigned int reading = 0;

  // counting number of low pulse in 1KHz
  if ( digitalRead(DUST_PWM_PIN) == LOW )
  {
    CntOfLP++;
  }


  TimesOfInt++;
  if (TimesOfInt >= 5000)  //output data every 1s
  {
    // moving average to achieve a smooth reading
    Filter.Update( CntOfLP ) ;
    // converting LPO to dust concentration
    t_x = Filter.GetAvg() / 50.0;
    Serial.print(",");
    Serial.print((int)t_x);
    t_x2 = t_x * t_x;
    reading = (PARAM_A * t_x2 * t_x + PARAM_B * t_x2 + PARAM_C * t_x + PARAM_D);

    Serial.print(",");
    Serial.print(reading);

#ifdef ENABLE_LCD
    // if ( reading != lastreading)
    {
      displayReading(reading, (int)t_x);
      //lastreading = reading;
    }
#endif //ENABLE_LCD  

    CntOfLP = 0; //reset counter

    Serial.println("");
    TimesOfInt = 0;
  }
  delay(1);
}

