#include "Wire.h"

#define I2CAddress 0x6D //0x28

void setup(){
      pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);  
  Serial.begin(19200);  // start serial for output
  Wire.begin();
  Serial.println("Amphenol Advanced Sensors");
  Serial.println("Arduino T9602 Monitor");
}

void getdata(byte *a, byte *b, byte *c, byte *d)
{
  Wire.beginTransmission(I2CAddress); //Sends the Measurement Request, Wire Library inserts '0' bit
  Wire.endTransmission();
  delay(50);                    //delay to measure and populate registers
 
  Wire.beginTransmission(I2CAddress); //initiate comms
  Wire.write(0);
  Wire.endTransmission();
  
  Wire.requestFrom(I2CAddress, 4);   //read registers
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


Serial.print(temperature);Serial.print(" degC  ");Serial.print(humidity);Serial.println(" %rH");;
}

void loop(){

  showthedata();
  
  delay(3000);
}
