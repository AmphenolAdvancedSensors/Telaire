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


