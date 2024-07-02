/*
 This program uses the DHT11 Temperature and Humidity Sensor, whos values are displayed 
 on A ILI9341 TFT screen, via the ESP32. The analogue meter(Humidity) was originally designed by Bodmer,
 I believe, and can be found in the TFT_eSPI example folder. I have re-codeed the meter using
 the Arduino_GFX_Library, and the Temperature meter was designed by me, and also uses the 
 Arduino_GFX_Library. Feel free to use and alter the code as needed!!
 */

#include "SPI.h"
#include <Arduino_GFX_Library.h>

#include "DHT.h"
#define DHTPIN 22    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

// Define the pins for the display based on your configuration
#define TFT_DC    2
#define TFT_CS    15
#define TFT_SCK   14
#define TFT_RST   -1     // Assuming no reset pin, set to -1 if not used
#define TFT_MOSI  13
#define TFT_MISO  12
#define TFT_BL    21     // Backlight pin

//Define Chip Select Pins, and Rotation
#define TS_CS 21  //7
#define SD_CS 5
#define ROTATION 0
uint32_t setUPTime = 0;       // time for next update

int my_Ntemp = 0;
int my_Otemp = 0;
int oBlock_H = 50;
int mH = 0, mH2 = 0;

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define DARK_GREY 0x4A89
#define MEDIUM_GREY 0x8492
#define LIGHT_GREY 0xD6BA
#define LIGHT_GREEN 0x970E

DHT dht(DHTPIN, DHTTYPE);

int redNeedle = 220, greyNeedle = 220,checkNeedle = 220;//Values set to 220,which equal 0 on meter.

Arduino_ESP32SPI bus = Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO);
Arduino_ILI9341 tft = Arduino_ILI9341(&bus, TFT_RST);

void setup() {
  Serial.begin(115200);
//*****Deselect all SPI Devices*****
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(21, OUTPUT);  //was 7
  digitalWrite(21, HIGH);  //was 7
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  //2 Lines below can be ignored, they're part of a bigger project
  pinMode(17, OUTPUT);
  digitalWrite(17, LOW);
  
  //tft.setRotation(uint8_t 1); 
  tft.begin();
  tft.setRotation(ROTATION);  
  
  dht.begin();
  myHumiditySetup();
  myTemperatureSetup();

  setUPTime = millis(); // Next update time
  Serial.print("Setup Time:     ");Serial.println(setUPTime);
}

//*********************************Loop Function Code**********************************
unsigned long lastFrame = millis();
void loop() {
  while((millis() - lastFrame) < 2000);
    lastFrame = millis();
    Serial.print("Loop lastFrame Time(millis()):     "); Serial.println(lastFrame);
   
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  Serial.print("Temperature(Celsius and Fahrenheit):     ");Serial.print((int) t);Serial.print("   "); Serial.println((int) f);
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
    } 
  mH2 = mH;  
  mH = (int) h;

  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(20, 90);
  tft.print(mH2);

  tft.setTextSize(2);
  tft.setTextColor(BLACK);  
  tft.setCursor(20, 90);
  tft.print(mH);
  
  temp_Update((int)t);    
  
  myMeter(mH + 220);
}

//*****************************Temperature Setup Function Code***************************
void myTemperatureSetup(){

  //**********Title Bar with Temperature Code********
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.setCursor(25, 130);
  tft.print("Temperature ");
  tft.setCursor(170, 130);
  tft.print(my_Ntemp);
  tft.print((char)247);
  tft.print("C");
  
  //*********Temperature Gauge Code********* 
  tft.fillRoundRect(tft.width()-135, tft.height()-170, 30, 150,20, BLACK);
  tft.fillRoundRect(tft.width()-132, tft.height()-167, 24, 150,20, WHITE);
  tft.fillCircle(tft.width()-120, tft.height()-30, 20, BLACK);
  tft.fillCircle(tft.width()-120, tft.height()-30, 17, WHITE);
  tft.fillRect(tft.width()-132, tft.height()-50, 24, 30,WHITE);
  tft.fillCircle(tft.width()-120, tft.height()-30, 12, RED);
  tft.fillRect(tft.width()-123, tft.height()-50, 6, 20,RED);

  //*********Temperature Gauge Lines Code**********
  int Line_Height = 270;
  for(int a = 0; a < 5; a++ ){
  tft.drawFastHLine(tft.width()-105,Line_Height,10,BLACK);
  Line_Height = Line_Height - 25;
  }

  int sLine_Height = 270;
  for(int b = 0; b < 20; b++ ){
  tft.drawFastHLine(tft.width()-105,sLine_Height,5,BLACK);
  sLine_Height = sLine_Height - 5;
  }

  //**********Temperature Gauge Numbers Code**********
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  
  int number_Height = 267;  //was 147
  int gNum = 0;
  for(int c = 0; c < 5; c++ ){
    tft.setCursor(tft.width()-92, number_Height); //was 225
    tft.print(gNum);
    tft.print((char)247);
    tft.print("C");
  number_Height = number_Height - 25;
  gNum = gNum + 25;
  }
}

//***********************Dial Gauge(Humidity) Setup Function Code************************
  
void myHumiditySetup(){
    tft.fillScreen(YELLOW);

    tft.fillRect(0, 0, 240, 126, DARK_GREY);
    tft.fillRect(5, 3, 230, 119, WHITE);
  
    tft.setTextSize(2);
    tft.setTextColor(BLACK);
    tft.setCursor(200, 90);
    tft.print("H%");
    tft.setCursor(20, 90);
    tft.print(mH);

    tft.drawArc(120, 140, 115, 99, 220, 320, BLACK);
    tft.fillArc(120, 140, 115, 100, 295, 319, YELLOW);
    tft.fillArc(120, 140, 115, 100, 270, 295, GREEN);
    tft.fillArc(120, 140, 115, 100, 220, 270, WHITE);
    
    tft.setTextSize(1);
    tft.setTextColor(BLACK);
    tft.setCursor(25, 57);
    tft.print("0");
    tft.setCursor(206, 56);
    tft.print("100");
    tft.setCursor(65, 25);
    tft.print("25");
    tft.setCursor(165, 25);
    tft.print("75");
    tft.setCursor(115, 15);
    tft.print("50");
  
    for (int a = 220; a < 320; a = a + 25){
      tft.fillArc(120, 140, 115, 100, a, a, BLACK);
    }
  
    for (int b = 225; b <= 315; b = b + 5){
      tft.fillArc(120, 140, 110, 100, b, b, BLACK);
    }
}

//***************************Dial Gauge Pointer Update Code***************************

void myMeter(int hNeedle){
//int redNeedle = 220, greyNeedle = 220,checkNeedle = 220, hNeedle = 220;
  checkNeedle = redNeedle; 
  if(hNeedle > redNeedle){
    redNeedle = hNeedle;
    for(int uRN = checkNeedle; uRN <= redNeedle; uRN++){
      tft.fillArc(120, 140, 95, 20, uRN - 1, uRN - 1, WHITE);
      tft.fillArc(120, 140, 95, 20, uRN, uRN, RED);
      tft.fillRect(95, 122, 48, 4, DARK_GREY);
      tft.fillRect(99, 126, 42, 3, YELLOW);
      delay(10);
   }    
 }else if(hNeedle < redNeedle){
    redNeedle = hNeedle;
    for(int dRN = checkNeedle; dRN >= redNeedle; dRN--){
      tft.fillArc(120, 140, 95, 20, dRN + 1, dRN + 1, WHITE);
      tft.fillArc(120, 140, 95, 20, dRN, dRN, RED);
      tft.fillRect(95, 122, 48, 4, DARK_GREY);
      tft.fillRect(99, 126, 42, 3, YELLOW);
      delay(10);
   } 
 }else{};

}

// ***************************Temperature Update Function Start***********************

//int oBlock_H = 50;
void temp_Update(int my_Ntemp){

  int nBlock_H;
  
  if(my_Ntemp != my_Otemp){
    tft.setTextSize(2);
    tft.setTextColor(YELLOW);
    tft.setCursor(170, 130);
    tft.print(my_Otemp);
    tft.print((char)247);
    tft.print("C");
    
    my_Otemp = my_Ntemp;
    tft.setTextSize(2);
    tft.setTextColor(BLACK);
    tft.setCursor(170, 130);
    tft.print(my_Ntemp);
    tft.print((char)247);
    tft.print("C"); 

    nBlock_H = map(my_Ntemp, 0, 100, 50, 150);
    Serial.print("nBlock: ");Serial.print(nBlock_H);Serial.print("     my_Ntemp: ");Serial.println(my_Ntemp);
 
    tft.fillRect(tft.width()-123, tft.height()- oBlock_H, 6, oBlock_H - 50,WHITE);
    tft.fillRect(tft.width()-123, tft.height()- nBlock_H, 6, nBlock_H - 50,RED);
    oBlock_H = nBlock_H;
    
    //To be commented out, or deleted
    Serial.print("Height: ");
    Serial.print(nBlock_H);
    Serial.print("  Temperature: ");
    Serial.print(my_Ntemp);
  }
 
}
