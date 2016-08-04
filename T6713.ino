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

#include "Wire.h"
#define T6713_I2C

//  Width Guide      "---------------------"
#define SplashScreen "T6713, v1.1"
char ScreenHeader[] = "   Carbon Dioxide";
char ScreenFooter[] = " Advanced Sensors";
char DisplayUnit[] = "ppm";

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "OELCD.h"

int k = 0;
int line = 1;

           //ppm CO2 corrected = ppm CO2 measured * ((Tmeasured*pref) / (pmeasured*Tref))

#ifdef T6713_I2C
#include "T6713.h"
#endif //T6713_I2C

           void setup()
{
  Serial.begin(115200);  // start serial for output
  Wire.begin();
  Serial.println(SplashScreen);
  Serial.print(line); Serial.print("\t");
  displaySetupScreen();
}


void loop() {

  GetCO2PPM();
  Serial.print(CO2ppmValue); Serial.print(DisplayUnit); Serial.print("\t");
  
  if ( k < 9 )
  {
    k++;
  } else {
    Serial.println("");
    k = 0;
    line++; Serial.print(line); Serial.print("\t");
  }

  displayReading(CO2ppmValue, DisplayUnit, ScreenHeader, ScreenFooter);
  delay(2000);
}


