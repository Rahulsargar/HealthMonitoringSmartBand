#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SparkFun_Si7021_Breakout_Library.h"

#define OLED_RESET 19 // RESET pin connected from ESP32 to OLED
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


float humidity = 0;
float tempC = 0;

//Create Instance of HTU21D or SI7021 temp and humidity sensor and MPL3115A2 barrometric sensor
Weather sensor;

void setup()   {                
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)

  // Clear the buffer.
  display.clearDisplay();

  //Initialize the I2C sensors and ping them
  sensor.begin();

}

void loop() {

  //Measure Relative Humidity from the HTU21D or Si7021
  humidity = sensor.getRH();

  //Measure Temperature from the HTU21D or Si7021
  tempC = sensor.getTemp();

  Serial.print("Temp:");
  Serial.print(tempC);
  Serial.print(" °C, ");

  Serial.print("Humidity:");
  Serial.print(humidity);
  Serial.println(" %");
  
  delay(10);

  // text display tests
  display.setTextSize(1);
  // text color
  display.setTextColor(WHITE);
  // set position
  display.setCursor(0,0);
  display.print("Temperature: ");
  display.print(tempC);
  display.println(" 'C");
  display.setCursor(0,15);
  display.print("Humidity: ");
  display.print(humidity);
  display.println(" %");
  display.display();
  delay(2000);
  display.clearDisplay();
}
