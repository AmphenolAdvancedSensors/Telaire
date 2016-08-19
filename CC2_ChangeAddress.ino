/*
  Programme changes address on T9602 or ChipCap2, their power needs to be derived from Output Pin 13
  to allow the criteria for getting into command mode to be controlled.
*/

#include "Wire.h"
int readline(int readch, char *buffer, int len)
{
  static int pos = 0;
  int rpos;

  if (readch > 0) {
    switch (readch) {
      case '\n': // Ignore new-lines
        break;
      case '\r': // Return on CR
        rpos = pos;
        pos = 0;  // Reset position index ready for next time
        return rpos;
      default:
        if (pos < len - 1) {
          buffer[pos++] = readch;
          buffer[pos] = 0;
        }
    }
  }
  // No end of line has been found, so return -1.
  return -1;
}

int OldI2CAddress = 0;
int NewI2CAddress = 0;
int i = 0;


byte data[10];

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);        //switches sensor on

  Serial.begin(115200);         // start serial for output
  while (!Serial);              // Leonardo: wait for serial monitor

  Wire.begin();
  Serial.println("Amphenol Advanced Sensors");
  Serial.println("Arduino I2C Change Address for ChipCap2 and T9602-D");

  Serial.print("Initial ");
  ScanI2CBus();
}

void loop() {
  static char buffer[80];

  if (OldI2CAddress <= 0 & i <= 0)
  {
    Serial.println("Enter Address to Change in Decimal");
    i = 1;
  }

  if (OldI2CAddress <= 0) {
    if (readline(Serial.read(), buffer, 80) > 0) {
      OldI2CAddress = atoi(buffer);
      Serial.print("You entered old: ");
      Serial.print(OldI2CAddress);
      Serial.println(" ");

    }
  }
  if (OldI2CAddress > 0 & NewI2CAddress <= 0 & i == 1) {
    Serial.println("Enter New Address in Decimal");
    i = 2;
  }

  if (OldI2CAddress > 0 & NewI2CAddress <= 0) {

    if (readline(Serial.read(), buffer, 80) > 0) {
      NewI2CAddress = atoi(buffer);
      Serial.print("You entered new: ");
      Serial.print(NewI2CAddress);
      Serial.println(" ");
    }
  }
  if (OldI2CAddress > 0 & NewI2CAddress > 0 & i == 2) {
    Serial.println("Are these values correct?");
    i = 3;
  }
  if (OldI2CAddress > 0 & NewI2CAddress > 0) {
    if (readline(Serial.read(), buffer, 80) > 0) {
      String answer = buffer;
      if (answer == "y" | answer == "Y" | answer == "yes" | answer == "Yes" | answer == "YES") {

        i = 4;
        //Sub routine to enter command mode, change address, and revert to normal mode, no reboot.
        ChangeAddress();
        Serial.println("Change Made");

        Serial.print("Before Reboot ");
        ScanI2CBus();

        //reboot sensor by power off-on
        digitalWrite(13, LOW); delay(100); digitalWrite(13, HIGH); delay(1);

        Serial.print("After Reboot ");
        ScanI2CBus();
      }
      else
      {
        i = 0;
        OldI2CAddress = 0 ;
        NewI2CAddress = 0;
      }
    }
  }



}


// --------------------Subroutines hereafter--------------------------------

void ChangeAddress() {
  digitalWrite(13, LOW);        //switches sensor off
  delay(5);
  digitalWrite(13, HIGH);        //switches sensor on
  delay(1);

  // Enter Command Mode
  Wire.beginTransmission(OldI2CAddress); Wire.write(0xA0); Wire.write(0x00); Wire.write(0x00);
  Wire.endTransmission();   // end transmission
  delay(1);

  // Read Status EEPROM Word 0x1C
  Wire.beginTransmission(OldI2CAddress); Wire.write(0x1C); Wire.write(0x00); Wire.write(0x00);
  Wire.endTransmission();   // end transmission
  delay(1);

  // data fetch of status
  Wire.requestFrom(OldI2CAddress, 3);
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();

  // print byte and calculated values
  // Serial.print(data[0]); Serial.print(","); Serial.print(data[1]); Serial.print(",");Serial.print(data[2]);
  // Serial.print("Address In Memory: 0x"); if (data[3] < 16){Serial.print("0");} Serial.println(data[3], HEX);

  //Write New Address to EEPROM
  Wire.beginTransmission(OldI2CAddress); Wire.write(0x5C); Wire.write(data[1]); Wire.write(NewI2CAddress);
  Wire.endTransmission();   // end transmission
  delay (12);
  
  // Set to Normal Mode
  Wire.beginTransmission(OldI2CAddress); Wire.write(0x80); Wire.write(0x00); Wire.write(0x00);
  Wire.endTransmission();   // end transmission
}

void ScanI2CBus()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address ");
      Serial.print(address, DEC); Serial.println(" (decimal)");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknow error at address ");
      Serial.println(address, DEC);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

}
