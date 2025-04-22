#include <Wire.h>
#include "Adafruit_AHTX0.h"

Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(9600);
  Serial.println("AHT20 test");

  if (!aht.begin()) {
    Serial.println("Could not find AHT? Check wiring!");
    while (1) delay(10);
  }

  Serial.println("AHT20 found");
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp); // Populate temp and humidity objects

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  delay(2000);
}
