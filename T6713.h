
#define ADDR_6713  0x15 // default I2C slave address

byte data[6];
int CO2ppmValue;

void GetCO2PPM()
{
  // start I2C
  Wire.beginTransmission(ADDR_6713);
  Wire.write(0x04); Wire.write(0x13); Wire.write(0x8B); Wire.write(0x00); Wire.write(0x01);
  // end transmission
  Wire.endTransmission();
  // read report of current gas measurement in ppm
  delay(2000);
  Wire.requestFrom(ADDR_6713, 4);    // request 4 bytes from slave device
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  data[3] = Wire.read();
  //  Serial.print("Func code: "); Serial.println(func_code);
  //  Serial.print("byte count: "); Serial.println(byte_count);
  //  Serial.print("MSB: ");  Serial.print(data[2]); Serial.print("  ");
  //  Serial.print("LSB: ");  Serial.print(data[3]); Serial.print("  ");
  CO2ppmValue = ((data[2] & 0x3F ) << 8) | data[3];
  //Serial.print(ppmValue);
  //return ppmValue;
}
