#include <Wire.h>             // Wire library (required for I2C devices)
#include <ESP8266WebServer.h> // ustawienia wifi
#include "Adafruit_GFX.h"     // Core graphics library
#include "Adafruit_ILI9341.h"  // Hardware-specific library for ST7789
#include "RTClib.h"           // RTC library
#include <Adafruit_BME280.h>   // Adafruit BME280 sensor library
#include  <SPI.h>

 
// ILI9341 TFT module connections
#define TFT_RST   RST     // TFT RST pin is connected to NodeMCU pin D8 (GPIO15)(--->4) 
#define TFT_DC    D4     // TFT DC  pin is connected to NodeMCU pin D4 (GPIO2)(--->5)
#define TFT_CS    D8     // TFT CS  pin is directly connected to GND (--->3)////-1
Adafruit_ILI9341 tft = Adafruit_ILI9341(15, 2); //

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
float elevation = 293.0;  // wysokość npm 
boolean sd_ok = 0, sensor_ok = 0; 
 

 
void setup()
{
   WiFi.mode(WIFI_STA); //wylaczenie rozglosu ssid
   
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  rtc.begin();         
  tft.begin();
  
 int SCREEN_WIDTH = 240; 
 int SCREEN_HEIGHT = 320; 
  // if the screen is flipped, remove this command
  tft.setRotation(0); //2
  // fill the screen with black color
  tft.fillScreen(ILI9341_BLACK);
  
  tft.fillRect(0, 32, tft.width(), 2, ILI9341_BLUE);
  tft.fillRect(0, 140, tft.width(), 2, ILI9341_BLUE);
  tft.fillRect(0, 170, tft.width(), 2, ILI9341_BLUE);
  tft.fillRect(0, 318, tft.width(), 2, ILI9341_BLUE);
  
  
  tft.setTextWrap(false);                        // turn off text wrap option
  tft.setTextSize(2);                 
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(46, 150);
  tft.print("POMIAR METEO:");
  // print degree symbol °C
  tft.setTextSize(3);
  tft.drawCircle(176, 196, 4, ILI9341_RED);
  tft.drawCircle(176, 196, 5, ILI9341_RED);
  tft.setCursor(186, 190); 
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);  
  tft.print("C  "); 
  // print symbol %
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setCursor(170, 235);
  tft.print(" %");
  // print symbol hPa
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);  
  tft.setCursor(175, 280); 
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
    tft.setTextSize(4);      // text size = 4
    tft.setCursor(3, 88);    // move cursor to position (3, 88) pixel
    tft.print("Connection");
    tft.setCursor(63, 126);  // move cursor to position (63, 126) pixel
    tft.print("Error");
    
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
  if( p_second != now.second() )
  {   // read & print sensor data every 1 second
    
    char buffer1[100];//22
    
    
    p_second = now.second();
    if( sensor_ok )
    {
  // read temperature and pressure from the BME280 sensor
  temp = bme280.readTemperature();  // get temperature in °C
  humi = bme280.readHumidity();     // get humidity in %
  pres = bme280.readPressure() / pow(2.718281828, -(elevation / ((273.15 + bme280.readTemperature()) * 29.263))) / 100.0F; // get pressure in Pa
 
  
  // 1: print temperature (in °C)
  //tft.setTextSize(3);
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);  
  tft.setCursor(43, 186);
  tft.print(temp);

  // 2: print humidity
   //tft.setTextSize(3);
   tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
   tft.setCursor(43, 230);
   tft.print(humi);
   //tft.print(" %");
    
  // 3: print pressure (in hPa)
  //tft.setTextSize(2);
  tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);  
  tft.setCursor(0, 275);
  tft.print(pres);
 // tft.print("hPa");
}
sprintf( buffer1, "%02u/%02u/%04u | %02u:%02u:%02u", now.day(), now.month(), now.year(),
                       now.hour(), now.minute(), now.second() );
 
    Serial.print(buffer1);
    if(sensor_ok) {
      
    }
    else
      Serial.println("  Error ");
   }
   delay(100);   // wait 100ms
}
//////////////////////////////////////// RTC functions ////////////////////////////////////////
void RTC_display()
{
   char _buffer[12];
   char dow_matrix[7][20] = {"NIEDZIELA", "PONIEDZIALEK", "WTOREK", "SRODA", "CZWARTEK", "PIATEK", "SOBOTA"};
  byte x_pos[7] = {65, 55, 80, 85, 60, 80, 80};
  static byte previous_dow = 8;
 
  // print day of the week
  if( previous_dow != now.dayOfTheWeek() )
  {
    previous_dow = now.dayOfTheWeek();
    tft.setTextSize(2);
    tft.fillRect(14, 0, 216, 28, ILI9341_BLACK);     // draw rectangle (erase day from the display)28
    tft.setCursor(x_pos[previous_dow], 10);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);     // set text color to cyan and black background
    tft.print( dow_matrix[now.dayOfTheWeek()] );
  }
 
  // print date
  sprintf( _buffer, "%02u-%02u-%04u", now.day()%100, now.month()%100, now.year() );
  tft.setTextSize(4);
  tft.setCursor(1, 50);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);  
  tft.printf(_buffer);
  // print time
  sprintf( _buffer, "%02u:%02u:%02u", now.hour()%100, now.minute()%100, now.second()%100 );
  tft.setCursor(26, 95);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);  
  tft.printf(_buffer);
}

byte edit(byte parameter)
{
  static byte i = 0, y_pos,
              x_pos[5] = {1, 73, 193, 26, 98};
 
  if(i < 3) {
    tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK); 
    y_pos = 50;
  }
  else {
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);  
    y_pos = 95;
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
 
      tft.setCursor(x_pos[i], y_pos);
      tft.printf("%02u", parameter);
      delay(200);       // wait 200 ms
    }
 
    tft.fillRect(x_pos[i], y_pos, 44, 28, ILI9341_BLACK);
    unsigned long previous_m = millis();
    while( (millis() - previous_m < 250) && digitalRead(button1) && digitalRead(button2)) ;
    tft.setCursor(x_pos[i], y_pos);
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
