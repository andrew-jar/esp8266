
#include <Wire.h>             // Wire library (required for I2C devices)
#include <ESP8266WebServer.h> // ustawienia wifi
#include <TFT_eSPI.h>   // https://github.com/Bodmer/TFT_eSPI
#include "RTClib.h"           // RTC library
#include <Adafruit_BME280.h>   // Adafruit BME280 sensor library
#include  <SPI.h>
 

TFT_eSPI tft = TFT_eSPI();

// initialize RTC library
RTC_DS3231 rtc;
DateTime   now;

// buttons definition
const int button1 = D0;  // button B1 is connected to NodeMCU D0 (GPIO16)
const int button2 = D1;  // button B2 is connected to NodeMCU D1 (GPIO5)
#define SEALEVELPRESSURE_HPA (1013.25)

#define BME280_I2C_ADDRESS  0x76
// initialize Adafruit BME280 library
Adafruit_BME280  bme280;
float temp, humi, pres;                                     
float elevation = 292.0;  // wysokość npm 
boolean sd_ok = 0, sensor_ok = 0; 

static const uint8_t temperature[] = {
  0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x00, 0x4c, 0x80, 0x00, 0x48, 0xc0, 0x18, 0x0a, 0x70, 0x09, 
  0xfa, 0x70, 0x03, 0x1a, 0x40, 0x06, 0x0a, 0x70, 0x04, 0x0a, 0xc0, 0x0c, 0x0a, 0x70, 0xec, 0x0a, 
  0xc0, 0x04, 0x0a, 0x40, 0x04, 0x13, 0x60, 0x02, 0x17, 0xa0, 0x01, 0xf7, 0x20, 0x08, 0x5a, 0x60, 
  0x18, 0x0c, 0xc0, 0x00, 0x47, 0x80, 0x00, 0x40, 0x00, 0x00, 0x40, 0x00
};
 
static const uint8_t humidity[] = {
  0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x60, 0x00, 0x00, 0xd0, 0x00, 0x01, 0x98, 0x00, 0x03, 
  0x0c, 0x00, 0x06, 0x06, 0x00, 0x04, 0x03, 0x00, 0x08, 0x01, 0x00, 0x18, 0x01, 0x80, 0x10, 0x00, 
  0x80, 0x10, 0x00, 0xc0, 0x14, 0x00, 0xc0, 0x14, 0x00, 0xc0, 0x16, 0x00, 0x80, 0x1b, 0x00, 0x80, 
  0x09, 0xf1, 0x00, 0x0c, 0xf3, 0x00, 0x07, 0x0e, 0x00, 0x01, 0xf8, 0x00
};

static const uint8_t epd_bitmap_icon_pressure[] = {
  0x03, 0xfc, 0x00, 0x0e, 0x67, 0x00, 0x18, 0x61, 0x80, 0x30, 0x00, 0xc0, 0x28, 0x01, 0x60, 0x40, 
  0x00, 0x20, 0xc0, 0x08, 0x30, 0x80, 0x18, 0x10, 0x80, 0x38, 0x10, 0xe0, 0x48, 0x70, 0xe0, 0x90, 
  0x70, 0x80, 0x90, 0x10, 0x80, 0x60, 0x10, 0xc0, 0x00, 0x30, 0x40, 0x00, 0x20, 0x60, 0x00, 0x60, 
  0x30, 0x00, 0xc0, 0x18, 0x01, 0x80, 0x0e, 0x07, 0x00, 0x03, 0xfc, 0x00
};
 
void setup()
{
   WiFi.mode(WIFI_STA); //wylaczenie rozglosu ssid

  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  rtc.begin();         
  //tft.begin();
  
  tft.init();
  tft.setRotation(0);
  // fill the screen with black color
  tft.fillScreen(ILI9341_BLACK);

  tft.drawLine(10, 38, 230, 38, ILI9341_BLUE);
  tft.drawRoundRect(1, 1, 239, 319, 8, ILI9341_BLUE);
  //tft.drawRoundRect(1, 1, 239, 134, 8, ILI9341_BLUE);
  //tft.drawRoundRect(1, 140, 239, 298, 8, ILI9341_BLUE);
  tft.drawRoundRect(6, 144, 228, 171, 8, ILI9341_BLUE); //x y w h
  //tft.fillRect(1, 38, tft.width(), 1, ILI9341_BLUE);
 // tft.fillRect(1, 138, tft.width(), 1, ILI9341_BLUE);
  //tft.fillRect(1, 318, tft.width(), 1, ILI9341_BLUE);
  //tft.fillRect(1, 1, tft.width(), 1, ILI9341_BLUE);
  //tft.drawRect(1, 1, 238, 318, ILI9341_BLUE);

  //tft.fillRect(0, 1, 240, 138, ILI9341_BLUE);
  //tft.fillRect(0, 1, tft.width(), 1, ILI9341_WHITE);

  tft.drawBitmap(22,190,temperature,20,20,ILI9341_CYAN); 
  tft.drawBitmap(22,234,humidity,20,20,ILI9341_CYAN);
  tft.drawBitmap(22,279,epd_bitmap_icon_pressure,20,20,ILI9341_CYAN); 
  
  tft.setTextWrap(false);                        // turn off text wrap option
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setCursor(36, 151, 4);
  tft.print("Pomiar Meteo:");
  // print degree symbol °C
  tft.drawCircle(186, 196, 4, ILI9341_WHITE);
  tft.drawCircle(186, 196, 5, ILI9341_WHITE);
  tft.setCursor(196, 190, 4); //186
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  
  tft.print("C  "); 
  // print symbol %
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(180, 235, 4);//170, 235
  tft.print(" %");
  // print symbol hPa
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  
  tft.setCursor(180, 280, 4); 
  tft.print("hPa");
 
  Wire.begin(D3, D2);  // set I2C pins [SDA = D3, SCL = D2], default clock is 100kHz ; initialize the BME280 sensor
  
   // initialize the BME280 sensor
  Serial.print("\r\nInitializing BME280 sensor...");
  if( bme280.begin(BME280_I2C_ADDRESS) )
  {  
     Serial.println("done.");
     sensor_ok = true;
  }
  else 
  {  // connection error or device address wrong!                                          
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);   // set text color to white and black background
    tft.setCursor(40, 210);    // move cursor to position (3, 88) pixel
    tft.print("Blad");
    tft.setCursor(40, 254);  // move cursor to position (63, 126) pixel
    tft.print("Polaczenia");


    // ----- Uzyskaj dane z czujnika BME280 i wyświetl zmierzone wartości na TFT -----
  temp = bme280.readTemperature();  // get temperature in °C
  humi = bme280.readHumidity();     // get humidity in %
  pres = bme280.readPressure() / pow(2.718281828, -(elevation / ((273.15 + bme280.readTemperature()) * 29.263))) / 100.0F; // get pressure in Pa
  
  }
} 


// main loop
void loop()
{
   now = rtc.now();  // read current time and date from the RTC chip
   RTC_display();   // display time & date
 
  if( !digitalRead(button1) )  // if B1 is pressed
  if( debounce() )             // call debounce function (make sure B1 is pressed)
  {
    while( debounce() );  // call debounce function (wait for B1 to be released)
 
    byte day    = edit( now.day() );          // edit date
    byte month  = edit( now.month() );        // edit month
    byte year   = edit( now.year() - 2000 );  // edit year
    byte hour   = edit( now.hour() );         // edit hours
    byte minute = edit( now.minute() );       // edit minutes
 
    // write time & date data to the RTC chip
    rtc.adjust(DateTime(2000 + year, month, day, hour, minute, 0));
 
    while(debounce());  // call debounce function (wait for button B1 to be released)
  }
 
  static byte p_second;
  if( p_second != now.second() )//second
  {   // read & print sensor data every 1 second
    
     
    
    p_second = now.second();
    if( sensor_ok )
    {
  // read temperature and pressure from the BME280 sensor
  temp = bme280.readTemperature();  // get temperature in °C
  humi = bme280.readHumidity();     // get humidity in %
  pres = bme280.readPressure() / pow(2.718281828, -(elevation / ((273.15 + bme280.readTemperature()) * 29.263))) / 100.0F; // get pressure in Pa

    // 1: print temperature (in °C)
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  
  tft.setCursor(95, 190, 4);//43,190
  tft.print(temp, 1);
  
  
  // 2: print humidity
   tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
   tft.setCursor(95, 234, 4);
   tft.print( humi,1 );
   
    
  // 3: print pressure (in hPa)
  tft.fillRect(60, 279, 81, 22, ILI9341_BLACK);   //X65, 279, X105, 22
  tft.fillRect(104, 279, 65, 22, ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); 
  if (pres < 1000) {
  tft.setCursor(80, 279, 4);  
  } else {
  tft.setCursor(68, 279, 4);  
  }
  tft.print(pres, 1);
 }
 
    if(sensor_ok) {
  }
    else
      Serial.println("  Error ");
   }
   //delay(100);   // wait 100ms
}
//////////////////////////////////////// RTC functions ////////////////////////////////////////
void RTC_display()
{
   
   
   char dow_matrix[7][20] = {"Niedziela", "Poniedzialek", "Wtorek", "Sroda", "Czwartek", "Piatek", "Sobota"};
  byte x_pos[7] = {65, 55, 80, 85, 70, 80, 80};
  static byte previous_dow = 8;

  // wydrukuj dzień tygodnia
  if( previous_dow != now.dayOfTheWeek() )
  {
    previous_dow = now.dayOfTheWeek();
    tft.fillRect(10, 8, 220, 30, ILI9341_BLACK);     // draw rectangle (erase day from the display)5, 3, 230, 35,
    tft.setCursor(x_pos[previous_dow], 10, 4);
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);     // set text color to cyan and black background
    tft.print( dow_matrix[now.dayOfTheWeek()] );


  }
 
  // print date
  tft.setCursor(48, 50, 4); //
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);  
  tft.printf("%02u / %02u / %04u", now.day()%100, now.month()%100, now.year());
  // print time
  tft.setCursor(26, 88, 6);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK); 
  tft.printf("%02u:%02u:%02u", now.hour()%100, now.minute()%100, now.second()%100);
}
byte edit(byte parameter)
{
  static byte i = 0, y_pos,
              x_pos[5] = {48, 95, 168, 27, 96}; //35, 102
              byte font_number; 
              byte  x_fill_rect; 
              byte  y_fill_rect; 

  if(i < 3) {
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); 
    font_number = 4;
    x_fill_rect = 28; 
    y_fill_rect = 28; 
    y_pos = 50;
  }
  else {
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    font_number = 6;
    x_fill_rect = 55;  // to trzeba zwiększyć
    y_fill_rect = 51; // to trzeba zwiększyć
    y_pos = 87; //93
   }
 
  while( debounce() );  // call debounce function (wait for B1 to be released)
 
  while(true) {
    while( !digitalRead(button2) ) {  // while B2 is pressed
      parameter++;
      if(i == 0 && parameter > 31)    // if day > 31 ==> day = 1
        parameter = 1;
      if(i == 1 && parameter > 12)    // if month > 12 ==> month = 1
        parameter = 1;
      if(i == 2 && parameter > 99)    // if year > 99 ==> year = 0
        parameter = 0;
      if(i == 3 && parameter > 23)    // if hours > 23 ==> hours = 0
        parameter = 0;
      if(i == 4 && parameter > 59)    // if minutes > 59 ==> minutes = 0
        parameter = 0;
 
      tft.setCursor(x_pos[i], y_pos,font_number);
      tft.printf("%02u", parameter);
      delay(200);       // wait 200 ms
    }
 
    tft.fillRect(x_pos[i], y_pos, x_fill_rect ,  y_fill_rect , ILI9341_BLACK);//28, 28
    unsigned long previous_m = millis();
    while( (millis() - previous_m < 250) && digitalRead(button1) && digitalRead(button2)) ;
    tft.setCursor(x_pos[i], y_pos, font_number); //,4
    
    
     
    tft.printf("%02u", parameter);
    previous_m = millis();
    while( (millis() - previous_m < 250) && digitalRead(button1) && digitalRead(button2)) ;
 
    if(!digitalRead(button1))
    {                     // if button B1 is pressed
      i = (i + 1) % 5;    // increment 'i' for the next parameter
      return parameter;   // return parameter value and exit
    }
  }
}

// a small function for button1 (B1) debounce
bool debounce ()
{
  byte count = 0;
  for(byte i = 0; i < 5; i++)
  {
    if ( !digitalRead(button1) )
      count++;
    delay(10);
  }
 
  if(count > 2)  return 1;
  else           return 0;
}
// end of code.
