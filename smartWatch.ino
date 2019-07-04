#include<DHTesp.h>
#include<RtcDS3231.h>
#include<TinyGPS++.h>
#include<ThingSpeak.h>
#include<ESP8266WiFi.h>
#include<Wire.h>
#include<SoftwareSerial.h>
#include<Adafruit_GFX.h>//Graphics library
#include<Adafruit_SSD1306.h>//Library to control driver chip(SSD1306) in OLED display

DHTesp dht;
TinyGPSPlus gps;

#define DHT11_Pin 2
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET LED_BUILTIN // Reset pin #
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);//making 'display' object. We don't need OLED_RESET but have to define it to declare the object 

static const int RXpin = 13;
static const int TXpin = 12;
static const int GPSbaud = 9600;
char *ssid = "FCTP";
char *pswd = "W!F!_k@_p@$$w0rd";
const char *api_key = "UDWGW2AFE8HCARQ8";
unsigned long int myChannelNumber = 812997;

SoftwareSerial ss(RXpin, TXpin);
WiFiClient client;
RtcDS3231<TwoWire> rtcObject(Wire);

static int uploadDelay = 0;
String today;
String current_time;
float Speed;

void setup()
{

  Serial.begin(9600);
  Wire.begin();
  rtcObject.Begin();
  RtcDateTime currentTime = RtcDateTime(19,07,04,21,23,30); //define date and time object
  rtcObject.SetDateTime(currentTime);                      //configure the RTC with object
  ss.begin(GPSbaud);
  //SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);//0x3C is I2C address of display
  dht.setup(DHT11_Pin, DHTesp::DHT11);//setup(uint8_t pin, DHT_MODEL_t model=AUTO_DETECT);
  WiFi.begin(ssid, pswd);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  ThingSpeak.begin(client);
  //rtc.setDOW(THURSDAY);     // Set Day-of-Week 
  //rtc.setTime(12, 0, 0);     // Set the time 
  //rtc.setDate(7, 4, 2019);   // Set the date 
  
  display.clearDisplay();//clear the display
  display.setTextColor(WHITE);//always use WHITE irrespective of actual color display
  display.setTextSize(1);//font size
}

void loop()
{
  
    float temp;
    float hum;
    if(uploadDelay >= 1200)
    {
      temp = dht.getTemperature();
      hum = dht.getHumidity();
      ThingSpeak.setField(1, temp);//int setField(unsigned int field, int value)
      ThingSpeak.setField(2, hum);
      ThingSpeak.writeFields(myChannelNumber, api_key);//int writeField(unsigned long channelNumber, unsigned int field, int value, const char * writeAPIKey)
      uploadDelay = 0; 
    }
    String strTemp = String(temp, 0);
    String strHum = String(hum, 0);
    display.setCursor(0, 40);
    display.print("Humidity: ");
    display.print(strHum);
    display.print("%");
    display.setCursor(0, 50);
    display.print("Temperature: ");
    display.print(strTemp);
    display.print("C");
    
    uploadDelay += 200;
    delay(200);
    
    if(ss.available() > 0)
    {
      gps.encode(ss.read());//have to repeatedly funnel the characters to TinyGPS++ from the GPS module using the encode() method to make it work.
      
      if(gps.location.isValid())//data goes to Google Maps API
      {
        double latitude = gps.location.lat();
        double longitude = gps.location.lng();
        
        Serial.print("latitude");
        Serial.println(latitude);
        Serial.print("longitude");
        Serial.println(longitude);

        if(gps.location.isUpdated())
        {
          Serial.println(latitude);
          Serial.println(longitude);
          ThingSpeak.setField(4, float(latitude));
          ThingSpeak.setField(5, float(longitude));
          ThingSpeak.writeFields(myChannelNumber, api_key);
        }
        ThingSpeak.setField(4, float(latitude));
        ThingSpeak.setField(5, float(longitude));
        ThingSpeak.writeFields(myChannelNumber, api_key);
      }
      
      RtcDateTime currentTime = rtcObject.GetDateTime();    //get the time from the RTC

//Display date using RTC DS3231   
      unsigned int Date = currentTime.Day();
      unsigned int Mon = currentTime.Month();
      unsigned int Yr = currentTime.Year();
      String strDate = String(Date);
      String strMonth = String(Mon);
      String strYear = String(Yr).substring(2, 4);
      today = strDate + "/" + strMonth + "/" + strYear;
      display.setCursor(0, 0);
      display.print("Date: ");
      display.print(today);


//Display time using RTC DS3231
      unsigned int Sec = currentTime.Second();
      unsigned int Min = currentTime.Minute();
      unsigned int Hr = currentTime.Hour();
      bool PMstatus = false;
      if(Hr > 12)
      {
        PMstatus = true;
        Hr -= 12;
      }
      String strSec = String(Sec);
      String strMinute = String(Min);
      String strHour = String(Hr);
      if(PMstatus)
      { 
        current_time = strHour + ":" + strMinute + ":" + strSec + " PM";
      }
      else
      {
        current_time = strHour + ":" + strMinute + ":" + strSec + " AM";
      }           
      display.setCursor(0, 10);
      display.print("Time: "); 
      display.print(current_time);

//If you want to use GPS to display date and time:
/*      
      if(gps.date.isValid())//data goes to OLED screen display
      {
        unsigned int date = gps.date.day();
        unsigned int month = gps.date.month();
        unsigned int year = gps.date.year();
        String strDate = String(date);
        String strMonth = String(month);
        String strYear = String(year).substring(2, 4);
        today = strDate + "/" + strMonth + "/" + strYear;
        
      }
      display.setCursor(0, 0);
      display.print("Date: ");
      display.print(today);

      Serial.print("today=");
      Serial.println(today);
*/
     
/*
      if(gps.time.isValid())//data goes to OLED screen display
      {
        unsigned int Hour = gps.time.hour();
        unsigned int Minute = gps.time.minute();
        Minute += 30;
        if(Minute > 60)
        {
          Minute -= 60;
          Hour += 1;
        }
        Hour += 5;
        bool PMstatus = false;
        if(Hour > 12)
        {
          PMstatus = true;
          Hour -= 12;
        }
        String strHour = String(Hour);
        String strMinute = String(Minute);
        if(PMstatus)
        { 
          currentTime = strHour + ":" + strMinute + " PM";
        }
        else
        {
          currentTime = strHour + ":" + strMinute + " AM";
        }
      }
      
      display.setCursor(0, 10);
      display.print("Time: ");
      display.print(currentTime);
      
      Serial.print("currentTime=");
      Serial.println(currentTime);
*/      
      if(gps.speed.isValid())//data goes to thingspeak.com
      {
        Speed = gps.speed.mps();
        ThingSpeak.setField(3, Speed);
        ThingSpeak.writeFields(myChannelNumber, api_key);
      }
      String strSpeed = String(Speed, 2); 
      display.setCursor(0, 30);
      display.print("Speed:  ");
      display.print(strSpeed);
      display.print("m/s");

      Serial.print("speed");
      Serial.println(strSpeed);
    
    }
    
    display.display();//to actually display the text
}
