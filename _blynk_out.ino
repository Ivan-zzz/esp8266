/*
BLYNK data transferring + oled 0.96" (i2c) indication
GPS neo-6m module + Wemos D1 mini (ESP8266)
by Ivan-Z `2021

Pin connection:
Wemos D1 mini - GPS neo6m - Oled  - buttons
-----------------------------------------------
5V            - 5v        - 5v    - nc (!!!) dont connect buttons with 5v to wemos D1 mini! its too much! 
3.3v          - nc        - nc    - Buttons 1,2...IN (previously I connected buttons with 5v, so fired-up 2 input ports, now they are always indicate high, working as output only, dont repeat my mistakes! :)))
GND           - GNC       - GND   - GND (with resistor 10k)
D2 (gpio4)sda - nc        - SDA   - nc
D1 (gpio5)scl - nc        - SCK   - nc
D5 (gpio14)   - TX        - nc    - nc
D6 (gpio12)   - RX        - nc    - nc
D0 (gpio16)   - nc        - nc    - button 1 (menu)
D7 (gpio13)   - nc        - nc    - button 2 (select). because of non-working input ports D3/D4 (which I melted by 5v input signal), its possible to manage all commands by only 1 button, and freeup HSPI 4 pins for sd card, for example. To avoid using 2nd button, select by short press, and menu change by long press...
usually gpio 12.13.14.15 are used for HSPI connections... so change buttonn pins to D3/D4 if you need SPI.
Good luck! ))

technical message from tutorial:
/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  App project setup:
    GPS Stream widget on V1.
 *************************************************************/

/* Comment this out to disable prints and save space */



//++++++++++++++++++++blynk needs+++++++++
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#define BLYNK_PRINT Serial // for debugging 
char auth[] = "_________________________"; // access token/id from blynk website
char ssid[] = "_________";  // any internet source
char pass[] = "_________";   // wifi password

//--------------------blynk needs---------------


#define LAST_SENTENCE_IN_INTERVAL NMEAGPS::NMEA_RMC //for gps percise time update
#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
#define menu D0    //Button 1 = menu
#define select D7  //Button 2 = select (as I mentioned before, you can manage all commands by using 1 button only (short press = select, long press = change next page)
int page = 0;      // select initial page
//home or base coordinates (maybe changed by pressing Button 2 (or short press to menu if you are using only 1 button scheme)
double Home_LAT0 = _____________; //if you need to setup home position and check distance from home
double Home_LNG0 = _____________; //if you need to setup home position and check distance from home 
double Home_LAT = 0; //if you need to setup home position and check distance from home
double Home_LNG = 0; //if you need to setup home position and check distance from home 


//sat20x20px logo
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
static const unsigned char u8g_logo_sat[] U8X8_PROGMEM = {

  
0x70,0x00,0x00,
0x70,0xF8,0x01,
0x70,0xFE,0x03,
0x80,0xFC,0x07,
0x00,0xF9,0x07,
0x40,0xF2,0x07,
0xC0,0xE4,0x07,
0xE0,0xE9,0x03,
0xE0,0xE3,0x03,
0xE0,0xFF,0x01,
0xE0,0xFF,0x01,
0xE0,0xFF,0x00,
0xE0,0x7F,0x00,
0xC0,0x1F,0x00,
0x80,0x07,0x00,
0x00,0x00,0x00,
0x00,0x00,0x00,
0x00,0x00,0x00,
0x00,0x00,0x00,

};
//wave10px logo
static const unsigned char u8g2_logo_wave[] U8X8_PROGMEM ={
  0xE0, 0x03, 0x18, 0x00, 0xC4, 0x01, 0x32, 0x00, 0x8A, 0x01, 0x69, 0x00, 
  0x25, 0x00, 0x95, 0x01, 0x95, 0x01, 0x01, 0x00, };




// (GPS device wiring ) - we can change D5 (14) / D6(12) pins to any other GPIO, to avoid using native RX/TX wemos pins, because they must be not connected while loading sketch, but as I said before, after firing-up  2 digital inputs on my wemos controller, I have some deficite of I/o pins, and obliged touse these tx/rx pins. you may change gps tx/rx pins to any free digital pin 
#include <SoftwareSerial.h>
static const int RXPin = 14, TXPin = 12; // use any digital i/o pins of D1 mini
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);

//settings for tiny GPS Library ----------
#include <TinyGPS++.h>
TinyGPSPlus gps;
double Lat;
double Long;
double Alt;
int day;
int month;
int year;
int num_sat; 
int gps_speed;
String heading; // direction, like S/NW/E etc
String bearing; // direction in degrees (if you like)
//int hour, minute, second; // if you wanna change time manually

boolean button  = false;           // button press indication
boolean press_flag = false;        // short press flag
boolean long_press_flag = false;   // long press flag
unsigned long last_press = 0;      // last time pressed
int sign = 1;                      // for indication of press results





void setup() {
  Serial.begin(9600);
  pinMode(menu, INPUT);  //for button 1
  pinMode(select, INPUT);//for button 2 if you use it
  ss.begin(GPSBaud);
   u8g2.begin();

//PrintingLoadingPage
    u8g2.firstPage();
  do {
    Loading_page();
  } while ( u8g2.nextPage() );
  delay(5000);


//-------------------------------Blynk CFG instead of instant connection ----
//  Blynk.begin(auth, ssid, pass); //identification type  - comment this line if you aren`t always connected
// You can also specify server:
//Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
//Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
Blynk.config(auth);
//Blynk.config(auth, server, port); // or with server/port

//---------------------------
}



//LOOP
void loop() {

  Get_GPS(); //Get GPS data
  if (digitalRead(menu) == HIGH)  // check if menu button pressed.
{
     page = (page+1);
//             else if (digitalRead(menu) == LOW)  //if you want to use menu button with auto-return to initial page
//             page = (page-1);                    //for returning to initial page as the button unpressed
  if (page<0 or page>3)
     page = 0;

}

   switch (page)  // several pages for small oled screen

{ 
  case 0: //page 1 clock
      u8g2.firstPage();
  do {
     Page_clock();  // first page
  } while ( u8g2.nextPage() );
  delay(10);
  break;

    case 1: // page 2 GPS location
  u8g2.firstPage();
  do {
    Page_location();
  } while ( u8g2.nextPage() );
  delay(10);
  break;

 case 2:  // page 3 destination point check + distance
     u8g2.firstPage();
  do {
    page_destination();

button = digitalRead(select) ;

if (button == true  && press_flag == false && millis() - last_press > 100) {
/*
 * if button wasnt pressed before, and mow pressed > 100ms
 */
    press_flag = !press_flag;       // short press flag = up
    last_press = millis();          // get last_press value (time)
  }
 
  if (button == true && press_flag == true && millis() - last_press > 2000) {
// if the button still remained pressed > 2 seconds
    long_press_flag = !long_press_flag;  // long press flag up
    last_press = millis();          // het last_press time
// Actions by this event (long press)
Home_LAT = Home_LAT0;
Home_LNG = Home_LNG0; 
sign = 3;
    

  }

  if (button == false && press_flag == true && long_press_flag == true) {
// released button (after long press)
    press_flag = !press_flag;            // short press flag down
    long_press_flag = !long_press_flag;  // long press flag down
  }

  if (button == false && press_flag == true && long_press_flag == false) {
// released button after short press
    press_flag = !press_flag;  // short press flag down
 // Actions by this event (short press)
Home_LAT = gps.location.lat();
Home_LNG = gps.location.lng(); 
sign = 2;

      }

u8g2.setFont(u8g2_font_courR08_tr);
u8g2.setCursor(0, 60);
if (sign == 1) {
u8g2.print("set destination");
}
if (sign == 2) {
u8g2.print("current pos = base");
}
if (sign == 3) {
u8g2.print("Destination = home");
}    // finishes signs



  }  // finished DO   
  while ( u8g2.nextPage() );
  delay(10);
  break;

  
case 3:  // page 4 speedometer
   u8g2.firstPage();
  do {
   Page_speed();
  } while ( u8g2.nextPage() );
  delay(10);
  break;


 } // end of switch


//+++++++++++++++++++++++++
Blynk.run ();  // I used this function for regular update of parameters. You can choose "connect" or "connectWIFI", or manual connection type.
//Blynk.connect();  // disconnects if you do nothing for keep connection constantly
//Blynk.connectWiFi(ssid, pass);

//check for wiring------------
  if (gps.charsProcessed() < 10)
  {
    Serial.println(F("GPS is not connected")); //warning to serial
      Blynk.virtualWrite(V1, "GPS is not connected");              //warning to blynk app
  }

//---------------------data transferring to blYnk
   Blynk.virtualWrite(V5, heading); //satellite_orientation
    Blynk.virtualWrite(V4, num_sat); //no_of_satellites
    Blynk.virtualWrite(V3, gps_speed); //gps_speed
    Blynk.virtualWrite(V1, String(Lat, 6)); //latitude   
    Blynk.virtualWrite(V2, String(Long, 6));  //longitude
  Blynk.virtualWrite(V6, Alt); // ???????????? ???? ????????


//-------------------------  


   
} //end of loop


void Loading_page() {
  u8g2.drawXBMP(0, 0, 20, 20, u8g_logo_sat);
//  u8g2.setFont( u8g2_font_crox1cb_tf);
u8g2.setFont(u8g2_font_helvB12_tf);
  //u8g2.setFont(u8g2_font_timB12_tf);
  u8g2.setCursor(35, 30);
  u8g2.print("GPS+Blynk");
  //u8g2.setFont(u8g2_font_7x13B_tf);
  u8g2.setFont(u8g2_font_nine_by_five_nbp_tf);
  u8g2.setCursor(45, 45);
  u8g2.print("by  Ivan");
  u8g2.setFont(u8g2_font_nine_by_five_nbp_tf);
  u8g2.setCursor(0, 60);
  u8g2.print("Loading");
  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  u8g2.setCursor(40, 60);
  u8g2.print(" . . . . . ");  
   
}  


void Page_clock()
  {
    u8g2.setFont(u8g2_font_courB08_tn);
    u8g2.setCursor(105, 64);
    u8g2.print( num_sat, 5);
    u8g2.drawXBMP(118, 54, 10, 10, u8g2_logo_wave);
    
    
    u8g2.setFont(u8g2_font_crox1cb_tf);
    u8g2.setCursor(20, 10);
    u8g2.print("GPS time");
    u8g2.drawLine(0,12,128,12);
    u8g2.setFont(u8g2_font_t0_22b_tn);
    u8g2.setCursor(20, 42);
    printTime(gps.time);
    // u8g.print(gps.date);
    //Get_Date();
    u8g2.setFont(u8g2_font_nine_by_five_nbp_tf);
    u8g2.setCursor(0, 64);
    printDate(gps.date);
  
  }
  
void Page_speed() {

  
 u8g2.setFont(u8g2_font_crox1cb_tf);
    u8g2.setCursor(16, 10);
    u8g2.print("Speedometer");
    u8g2.drawLine(0,15,128,15);
    
  u8g2.setFont(u8g2_font_t0_22b_tn);
  u8g2.setCursor(5, 42);
  u8g2.print(gps_speed , DEC);
  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  u8g2.setCursor(62, 42);
  u8g2.print("km/h");


  u8g2.setFont(u8g2_font_courB08_tn);
  u8g2.setCursor(105, 64);
  u8g2.print( num_sat, 5);
  u8g2.drawXBMP(118, 54, 10, 10, u8g2_logo_wave);

  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  u8g2.setCursor(0,64);
  u8g2.print("Direction:");
  u8g2.setCursor(45,64);
  u8g2.print( heading);
}

void Page_location()
{
  u8g2.setFont(u8g2_font_crox1cb_tf);
  u8g2.setCursor(10, 10);
  u8g2.print("GPS Location");
  u8g2.drawLine(0,12,128,12);

  u8g2.setFont(u8g2_font_nine_by_five_nbp_tf);
  u8g2.setCursor(5, 28);
  u8g2.print("Long: ");
  u8g2.setCursor(40, 28);
  u8g2.print( Long, 6);

  u8g2.setCursor(5, 43);
  u8g2.print("Lat: ");
  u8g2.setCursor(40, 43);
  u8g2.print( Lat, 6);

  u8g2.setCursor(0, 64);
  u8g2.print("Alt: ");
  u8g2.setCursor(20, 64);
  u8g2.print( Alt, 3);

  u8g2.setFont(u8g2_font_courB08_tn);
  u8g2.setCursor(105, 64);
  u8g2.print( num_sat, 5);
  u8g2.drawXBMP(118, 54, 10, 10, u8g2_logo_wave);
  
  }

// This custom version of delay() ensures that the gps object is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}


void Get_GPS()
{


  num_sat = gps.satellites.value();

  if (gps.location.isValid() == 1) {

    Lat = gps.location.lat();
    Long = gps.location.lng();
    Alt = gps.altitude.meters();
    gps_speed = gps.speed.kmph();

   heading = gps.cardinal(gps.course.value());  // as standard directions W/E/S/N
   bearing = gps.course.deg(); // get the direction - in degrees

  }


//Manual setup of time/date, if GPS cant be obtained
 /*   if (gps.date.isValid())
    {
      day = gps.date.day();
      month = gps.date.month();
      year = gps.date.year();
    }
  if (gps.time.isValid())
    {

      hour = gps.time.hour()+3;
      minute = gps.time.minute();
      second = gps.time.second();
    }
*/

//

  smartDelay(1000);


  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
     Serial.println(F("GPS is not connected"));
           Blynk.virtualWrite(V1, "GPS is not connected");  

  }



}

static void printDate(TinyGPSDate &d)
{
  if (!d.isValid())
  {
u8g2.print(F("******** "));

  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d.%02d.%02d ", d.day(), d.month(), d.year());
    u8g2.print(sz);
  }
}
static void printTime(TinyGPSTime &t)
{  
  if (!t.isValid())
  {
    u8g2.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour()+3, t.minute(), t.second());// hour +3 correction (for Moscow)
    u8g2.print(sz);
  }

 // printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

void page_destination()
{
unsigned long distancem =
  (unsigned long)TinyGPSPlus::distanceBetween(
    gps.location.lat(),
    gps.location.lng(),
    Home_LAT,
    Home_LNG );
      u8g2.setFont(u8g2_font_nine_by_five_nbp_tf);
      u8g2.setCursor(0, 20);
      u8g2.print("Distance: ");
      u8g2.setCursor(50, 20);
      u8g2.print(distancem);
      u8g2.setCursor(90, 20);
      u8g2.print(" m");
      
double courseTo =
//unsigned long courseTo =
  TinyGPSPlus::courseTo(
    gps.location.lat(),
    gps.location.lng(),
    Home_LAT,
    Home_LNG );
      
      u8g2.setCursor(0, 30);
      u8g2.print("Course to: ");
      u8g2.setCursor(60, 30);
      u8g2.print(courseTo);
 //     u8g2.setCursor(90, 30);
 //     u8g2.print(" m");

String cardinalTo = TinyGPSPlus::cardinal(courseTo);
      u8g2.setCursor(0, 40);
      u8g2.print("Cardinal: ");
      u8g2.setCursor(60, 40);
      u8g2.print(cardinalTo);


  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  u8g2.setCursor(0,50);
  u8g2.print("Head:");
  u8g2.setCursor(25,50);
  u8g2.print( heading);

  u8g2.setFont(u8g2_font_glasstown_nbp_tf);
  u8g2.setCursor(65,50);
  u8g2.print("Bear:");
  u8g2.setCursor(95,50);
  u8g2.print( bearing);



}
