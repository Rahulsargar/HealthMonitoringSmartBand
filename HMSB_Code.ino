#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Time.h>
#include <DS1307RTC.h>
#include "SparkFun_Si7021_Breakout_Library.h"
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#define OLED_RESET 4

float humidity = 0;
float tempC = 0;
Weather sensor;
MAX30105 particleSensor;
SSD1306AsciiWire oled;

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
//Arduino Uno doesn't have enough SRAM to store 50 samples of IR led data and red led data in 32-bit format
//To solve this problem, 16-bit MSB of the sampled data will be truncated. Samples become 16-bit data.
uint16_t irBuffer[50]; //infrared LED sensor data
uint16_t redBuffer[50];  //red LED sensor data
#else
uint32_t irBuffer[50]; //infrared LED sensor data
uint32_t redBuffer[50];  //red LED sensor data
#endif
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

  void setup()
{ 
   Serial.begin(9600);
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
   display.display();
   display.clearDisplay();
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.clearDisplay();
   //Initialize the I2C sensors and ping them
   sensor.begin();

  if(!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while(1);
  }
  particleSensor.setup(55, 4, 2, 200, 411, 4096); //Configure sensor with these settings  
}

  void loop()
{
    display.clearDisplay();
    tmElements_t tm;
    if(RTC.read(tm)){
      display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(5,8);
    print2digits(tm.Hour);
    display.write(':');
    print2digits(tm.Minute);
    display.setTextSize(2);
    display.setCursor(100,8);
     print2digits(tm.Second);
    display.setCursor(16,45);
    display.setTextSize(2);
    Serial.println(tm.Day);
    display.print(tm.Day);
    display.write('/');
    Serial.println(tm.Month);
    display.print(tm.Month);
    display.write('/');
    Serial.println(tmYearToCalendar(tm.Year));
    display.print(tmYearToCalendar(tm.Year));
    display.display();

    delay(4000);
    display.clearDisplay();

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
    
    delay(4000);
    display.clearDisplay();

    for (byte i = 0 ; i < 50 ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
    particleSensor.check(); //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }
  //calculate heart rate and SpO2 after first 50 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, 50, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    //dumping the first 25 sets of samples in the memory and shift the last 25 sets of samples to the top
    for (byte i = 25; i < 50; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }
    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 25; i < 50; i++)
    {
      while (particleSensor.available() == false) //do we have new data?
        particleSensor.check(); //Check the sensor for new data
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample
      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);
      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);
      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);
      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);
      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC); 
    }
    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, 50, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
    // text display tests
    display.setTextSize(1);
    // text color
    display.setTextColor(WHITE);
    // set position
    display.setCursor(0,0);
    display.print("HR: ");
    display.print(heartRate, DEC);
    display.setCursor(0,15);
    display.print("SPO2: ");
    display.print(spo2, DEC);
    display.display();
    
    delay(4000);
    display.clearDisplay();
}
