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

void setup(){
  Serial.begin(115200);  // start serial for output
  Wire.begin();
  Serial.println("Amphenol Advanced Sensors");
  Serial.println("Arduino T9602 Monitor");
}

void getdata(byte *a, byte *b, byte *c, byte *d)
{
  Wire.beginTransmission(0x00);
  Wire.write(0);
  Wire.endTransmission();
  
  Wire.requestFrom(0x00, 4);
  *a = Wire.read();
  *b = Wire.read();
  *c = Wire.read();
  *d = Wire.read();
}
 void showthedata()
{
  byte aa,bb,cc,dd;
  float temperature=0.0;  float humidity=0.0;
  getdata(&aa,&bb,&cc,&dd);
 
// humidity = (rH_High [5:0] x 256 + rH_Low [7:0]) / 16384 x 100
  humidity = (float)(((aa & 0x3F ) << 8) + bb) / 16384.0 * 100.0;

// temperature = (Temp_High [7:0] x 64 + Temp_Low [7:2]/4 ) / 16384 x 165 - 40
  temperature = (float)((unsigned)(cc  * 64) + (unsigned)(dd >> 2 )) / 16384.0 * 165.0 - 40.0;

//static void cc2_calculations(void)
//{
//  raw_rh =
//    (uint32_t)(((uint16_t)(i2c_buf[0] & 0x3F) * 256) + (uint16_t)i2c_buf[1]);
//  cooked_rh = ((double)raw_rh / 16384.0) * 100.0;

//  raw_tc =
//    (int32_t)(((uint16_t)i2c_buf[2] * 64) + ((uint16_t)(i2c_buf[3] & 0xFC) >> 2));
//  cooked_tc = ((double)raw_tc / 16384.0) * 165.0 - 40.0;
//} // cc2_calculations



Serial.print(temperature);Serial.print(" degC  ");Serial.print(humidity);Serial.println(" %rH");;
}

void loop(){

  showthedata();
  
  delay(3000);
}
