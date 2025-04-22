#include <Wire.h>
#include "rgb_lcd.h"
#include <Arduino.h>
#include <pins_arduino.h>
#include "Adafruit_AHTX0.h"
#include <math.h>

Adafruit_AHTX0 aht;

#define DHTTYPE DHT11
#define DHTPIN SDA

void colorAtTemp(float temp);

rgb_lcd lcd;

int colorR = 0;
int colorG = 255;
int colorB = 0;

float min_temp = 10;
float eq_temp = 20;
float max_temp = 40;

void setup() 
{
  Serial.begin(9600);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  
  lcd.setRGB(colorR, colorG, colorB);
  
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("Weather Project");
  lcd.setCursor(0, 1);
  lcd.print("Initializing");
  for(int i = 0; i<3; i++){
    delay(1000);
    lcd.print(".");
  }
  delay(500);

  if (!aht.begin()) {
    Serial.println("Could not find AHT? Check wiring!");
    while (1) delay(10);
  }

  delay(1000);
}

void loop() {

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  colorAtTemp(temp.temperature);

  lcd.setRGB(colorR, colorG, colorB);

  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.print("Temp = ");
  lcd.print(temp.temperature);
  lcd.print("C"); 
  lcd.setCursor(0, 1);
  lcd.print("Hum = "); 
  lcd.print(humidity.relative_humidity);
  lcd.print("%"); 

  Serial.print(millis()/1000);
  Serial.print(","); // time
  Serial.print(temp.temperature);
  Serial.print(","); // temp
  Serial.println(humidity.relative_humidity); // humidity


  delay(50);
}

void colorAtTemp(float temp){
  if (temp > eq_temp){
    colorR = 255;
    colorB = colorG = 255*pow(1-(temp-eq_temp)/(max_temp-eq_temp),2);
  
    if (temp>max_temp){
      colorB = colorG = 0;
    }
  }else{
    colorB = 255;
    colorR = colorG = 255*pow(1-(eq_temp-temp)/(eq_temp-min_temp),2);

    if (temp<min_temp){
      colorR = colorG = 0;
    }
  }
}
