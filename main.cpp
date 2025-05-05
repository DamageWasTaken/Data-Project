#include <Wire.h>
#include "rgb_lcd.h"
#include <Arduino.h>
#include <pins_arduino.h>
#include "Adafruit_AHTX0.h"
#include <math.h>
#include <SPI.h>
#include <SD.h>

Adafruit_AHTX0 aht;

#define DHTTYPE DHT11
#define DHTPIN SDA

void colorAtTemp(float temp);
void tempIcon(float temperature);
void humiIcon(float humidity);

rgb_lcd lcd;

int colorR = 0;
int colorG = 255;
int colorB = 0;


const short cd_pin = 9;

const float min_temp = 20;
const float eq_temp = 25;
const float max_temp = 30;

byte celsius[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000,

};

byte drenched[8] = {
  0b00000,
  0b00100,
  0b00100,
  0b01110,
  0b01110,
  0b11111,
  0b11111,
  0b01110,
};

byte pretty_humid[8] = {
  0b00000,
  0b00100,
  0b00100,
  0b01010,
  0b01010,
  0b10111,
  0b11111,
  0b01110,

};

byte almost_humid[8] = {
  0b00000,
  0b00100,
  0b00100,
  0b01010,
  0b01010,
  0b10011,
  0b10111,
  0b01110,

};

byte very_dry[8] = {
  0b00000,
  0b00100,
  0b00100,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b01110,

};

byte too_hot[8] = {
  0b00100,
  0b01010,
  0b01100,
  0b01010,
  0b01110,
  0b11111,
  0b11111,
  0b01110,
};

byte just_enough[8] = {
  0b00100,
  0b01010,
  0b01100,
  0b01010,
  0b01010,
  0b10111,
  0b11111,
  0b01110,
};

byte too_cold[8] = {
  0b00100,
  0b01010,
  0b01100,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b01110,
};



struct format{
  char delimiter;
  char endline;
  char decimal_sepperator;
  String filename;
  String filepath;
  File* file;
};

format formatDK;
File dataDK;

format formatInter;
File data;

format* formats[2] = {&formatDK, &formatInter};


void setup() {
  Serial.begin(9600);
  while (!Serial);

  // DK
  formatDK.delimiter = ';';
  formatDK.endline = '\n';
  formatDK.decimal_sepperator = ',';
  formatDK.filename = "datadk.txt";
  formatDK.file = &dataDK;


  //International
  formatInter.delimiter = ',';
  formatInter.endline = '\n';
  formatInter.decimal_sepperator = '.';
  formatInter.filename = "data.txt";
  formatInter.file = &data;

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  
  lcd.setRGB(colorR, colorG, colorB);

  lcd.createChar(0, celsius);
  lcd.createChar(1, drenched);
  lcd.createChar(2, pretty_humid);
  lcd.createChar(3, almost_humid);
  lcd.createChar(4, very_dry);

  lcd.createChar(5, too_hot);
  lcd.createChar(6, just_enough);
  lcd.createChar(7, too_cold);
  
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("Weather Project");
  lcd.setCursor(0, 1);
  lcd.print("Initializing");
  for(int i = 0; i<3; i++){
    delay(1000);
    lcd.print(".");
  }
  if(!SD.begin(cd_pin)){
    lcd.setRGB(255,0,0);
    lcd.setCursor(0, 1);
    lcd.print("Failed - No SD");
    while(!SD.begin(cd_pin));
  }
  delay(500);

  if (!aht.begin()) {
    Serial.println("Could not find AHT? Check wiring!");
    while (1) delay(10);
  }

  static String readings_folder = "/Reading";
  int current = 0;

  while (true){
    if(SD.exists(readings_folder+String(current))){
      current++;
      continue;
    } else {
      SD.mkdir(readings_folder+String(current));
      readings_folder += String(current);
      break;
    }
  }
  for (short i = 0; i<sizeof(formats)/sizeof(formats[0]);i++){
    formats[i]->filepath = readings_folder + '/' + formats[i]->filename;

    if(SD.exists(formats[i]->filepath)){
      SD.remove(formats[i]->filepath);
    }
    
    *formats[i]->file = SD.open(formats[i]->filepath, FILE_WRITE);
    formats[i]->file->close();
  }
  delay(1000);
}

unsigned long start = millis();
unsigned long rep_interval = 10000;
unsigned long last_interval = millis();
unsigned long now = millis();

struct {
  short reads;
  float temp;
  float humid;
} avr_pool;

void loop() {
  now = millis();
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  colorAtTemp(temp.temperature);

  lcd.setRGB(colorR, colorG, colorB);

  lcd.clear();
  lcd.setCursor(0, 0);

  tempIcon(temp.temperature);

  lcd.print("Temp = ");
  lcd.print(temp.temperature);
  lcd.write((unsigned char)0);
  lcd.print("C"); 
  
  lcd.setCursor(0, 1);
  humiIcon(humidity.relative_humidity);
  lcd.print("Humi = "); 
  lcd.print(humidity.relative_humidity);
  lcd.print("%"); 

  Serial.print(millis()/60000);
  Serial.print(","); // time
  Serial.print(temp.temperature);
  Serial.print(","); // temp
  Serial.println(humidity.relative_humidity); // humidity

  if(!SD.begin(cd_pin)){
    lcd.setRGB(255,0,0);
    lcd.setCursor(0, 0);
    lcd.print("Weather Project");
    lcd.setCursor(0, 1);
    lcd.print("Failed - No SD");
    while(!SD.begin(cd_pin));
  }

  Serial.println(now - last_interval);
  if (now - last_interval >= rep_interval){
    for (unsigned short i = 0; i<static_cast<unsigned short>(sizeof(formats)/sizeof(formats[0]));i++){
      *formats[i]->file = SD.open(formats[i]->filepath, FILE_WRITE);
      String fileline = String(static_cast<float>(now-start)/60000)+formats[i]->delimiter+String(avr_pool.temp/avr_pool.reads)+formats[i]->delimiter+String(avr_pool.humid/avr_pool.reads)+formats[i]->endline;
      if (formats[i]->decimal_sepperator != '.'){
        for(short n = 0; n<fileline.length(); n++){
          if (fileline[n] == '.'){
            fileline[n] = formats[i]->decimal_sepperator;
          }
        }
      }
      formats[i]->file->print(fileline);
      formats[i]->file->close();
    }
    last_interval = millis();
    avr_pool.humid = 0;
    avr_pool.temp = 0;
    avr_pool.reads = 0;
  }

  avr_pool.humid += humidity.relative_humidity;
  avr_pool.temp += temp.temperature;
  avr_pool.reads++;

  delay(50);
}

void tempIcon(float temperature){
  if (temperature<=min_temp){
    lcd.write((unsigned char)7);
  } else
  if (min_temp<temperature && temperature<=max_temp){
    lcd.write((unsigned char)6);
  } else
  if (temperature>max_temp){
    lcd.write((unsigned char)5);
  }
}

void humiIcon(float humidity){
  if (humidity<=25){
    lcd.write((unsigned char)4);
  } else
  if (25<humidity && humidity<=50){
    lcd.write((unsigned char)3);
  } else
  if (50<humidity && humidity<=75){
    lcd.write((unsigned char)2);
  } else
  if (humidity>75){
    lcd.write((unsigned char)1);
  }
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
