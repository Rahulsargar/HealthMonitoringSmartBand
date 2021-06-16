//Introduce Si7021
#include "Adafruit_Si7021.h"
#include <SoftwareSerial.h>

SoftwareSerial BTserial(2, 3); // RX | TX

Adafruit_Si7021 sensor = Adafruit_Si7021();
float hum=0,tem=0;

//Introduce MAX30102
#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 2; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(4800);
  BTserial.begin(9600);
  // wait for serial port to open
  while (!Serial) {
    delay(10);
  }
  Serial.println("Si7021 test!");
  if (!sensor.begin()) {
    Serial.println("Did not find Si7021 sensor!");
    while (true);
  }

  //Initialise MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }


 
  Serial.println("Place your index finger on the sensor with steady pressure.");
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void loop() {

  Humtemp();

  Heart();

}
void Humtemp(){
  hum=sensor.readHumidity();
  tem=sensor.readTemperature();
  Serial.print("Humidity:    ");
  Serial.print(hum, 2);
  Serial.print("Temperature: ");
  Serial.println(tem, 2);
  BTserial.print("Humidity: ");
  BTserial.print(hum);
  BTserial.print("%");

  BTserial.print("Temperature: ");
  BTserial.print(tem);
  BTserial.print("'C");
}

void Heart(){

  //Calculate Heart Rate
 long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 0)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  BTserial.print(F("HR: "));
  BTserial.print(beatAvg);


  if (irValue < 50000)
    Serial.print(" No finger?");
}
