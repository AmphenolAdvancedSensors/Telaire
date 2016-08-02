
//set up LCD

#define OLED_MOSI   11 //SDA
#define OLED_CLK   12 //SCL
#define OLED_DC    9 // D/C
#define OLED_CS     11// not used
#define OLED_RESET 10 //RESET
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void displaySetupScreen()
{
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 30);
  display.print(SplashScreen);
  display.display();
  delay(2000);
  display.clearDisplay();
}

void displayReading(int aa, char* bb, char* cc, char* dd) {
  display.clearDisplay();
  
// display header detail
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(cc);

// display reading and units 
  display.setTextSize(3);
  display.setCursor(15, 20);
  display.print(aa);
  display.setCursor(85, 20);
  display.setTextSize(2);
  display.print(bb);
  
// display footer detail
  display.setCursor(10, 57);
  display.setTextSize(1);
  display.print(dd);
  
  display.display();
}

