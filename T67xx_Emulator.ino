/* Arduino UNO or compatible Board
 * I2C Output on Terminals SDA/SCL, responds as a T67x3 series CO2 Module
 * 
 * 0.96" SPI OLED or similar wired as:
 * Gnd - Gnd
 * Vcc 13
 * SCL 12
 * SDA 11
 * RES 10
 * DC 9
 * 
 * 10kΩ ~ 50kΩ potentiometer wired between 5V and ground, output to A0
 * 
 * Multiplier Circuit Switches, standard output is 0 - 1024ppm
 * Take Digital Input 2 to ground to multiply by 2 (0 - 2048)
 * Take Digital Input 3 to ground to multiply by 4 (0 - 4096)
 * Take Digital Input 4 to ground to multiply by 8 (0 - 8192)
*/

// Include libraries for display and communications
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define version "T67xx I2C Emulator"
#define I2CAddress 0x15  //default i2c for T67xx sensor

//(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS)
Adafruit_SSD1306 display(128, 64, 11, 12, 9, 10, 8);
#define VccPin 13


void display_prepare();
void displayReading(int V);
void requestEvent();

int CO2Value = 400;
byte multiplier = 1;

void setup(void) { //-------------------------SetUp--------------------------

  pinMode(VccPin, OUTPUT); //Switches Screen On
  digitalWrite(VccPin, HIGH);

  pinMode(2, INPUT_PULLUP); //2 x multiplier
  pinMode(3, INPUT_PULLUP); //4 x multiplier
  pinMode(4, INPUT_PULLUP); //8 x multiplier

  display.setRotation(0);
  display_prepare();
  display.setCursor(10, 30);
  display.print(version);
  display.display();  delay(1500);

  Wire.begin(I2CAddress);// join i2c bus with address
  Wire.onRequest(requestEvent); // register event
}

void loop(void) {//------------------------Loop---------------------------
  multiplier = 1;
  CO2Value = analogRead(A0);
  if (!digitalRead(2)) multiplier = 2;
  if (!digitalRead(3)) multiplier = 4;
  if (!digitalRead(4)) multiplier = 8;

  CO2Value = CO2Value * multiplier;
  displayReading(CO2Value);
  delay(500);
}

//-----------------------------------------Display Routines--------------------
void display_prepare(void) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void displayReading(int V) {
  float Variable;
  //char SensorReading[4] = itoa(data[1] | data[0] << 8);
  display_prepare();

  display.setCursor(0, 5);  display.print(F("Emulated Value"));
  display.setCursor(100, 55);  display.print(F("ppm"));
  display.setTextSize(4);
  display.setCursor(25, 22); display.print(V);

  display.display();
}
//________________________________I2C RESPONSE_______________________________

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  byte data[4];
  data[0] = (0);          //MSB
  data[1] = (0);          //LSB
  data[2] = ((CO2Value & 0xFF00) >> 8);         //MSB
  data[3] = (CO2Value & 0x00FF);               //LSB

  Wire.write(data[0]);
  Wire.write(data[1]);
  Wire.write(data[2]);
  Wire.write(data[3]);

}
