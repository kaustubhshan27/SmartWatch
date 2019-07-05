#include<DHTesp.h>
#include<TinyGPS++.h>
#include<ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include<SoftwareSerial.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Adafruit_GFX.h>//Graphics library
#include <Adafruit_SSD1306.h>//Library to control driver chip(SSD1306) in OLED display

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
char *days[] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};

WiFiUDP ntpUDP;//for UDP pakcet transfers 
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800,60000);//"pool.ntp.org" gives servers from worldwide, UTC offset in seconds
SoftwareSerial ss(RXpin, TXpin);
WiFiClient client;

static int uploadDelay = 0;
String today;
String current_time;
String day_of_week;
float Speed;
float temp;
float hum;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  ss.begin(GPSbaud);
  //SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);//0x3C is I2C address of display
  dht.setup(DHT11_Pin, DHTesp::DHT11);//setup(uint8_t pin, DHT_MODEL_t model=AUTO_DETECT);
  WiFi.begin(ssid, pswd);
  timeClient.begin();

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

  display.clearDisplay();//clear the display
  display.setTextColor(WHITE);//always use WHITE irrespective of actual color display
  display.setTextSize(1);//font size
}

void loop()
{
    timeClient.update();//to receive date and time from NTP servers
    display.clearDisplay();//clear the display
  
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

        if(gps.location.isUpdated())
        {
          ThingSpeak.setField(4, float(latitude));
          ThingSpeak.setField(5, float(longitude));
          ThingSpeak.writeFields(myChannelNumber, api_key);
        }
        ThingSpeak.setField(4, float(latitude));
        ThingSpeak.setField(5, float(longitude));
        ThingSpeak.writeFields(myChannelNumber, api_key);
      }

//NTP servers for date and time:
      day_of_week = days[timeClient.getDay()];
      display.setCursor(0, 0);
      display.print(day_of_week);

      today = timeClient.getFormattedDate().substring(2, 10);

      String strDate = today.substring(6, 8);
      String strMonth = today.substring(3, 5);
      String strYear = today.substring(0, 2);
      today = strDate + "/" + strMonth + "/" + strYear;
      display.setCursor(0, 10);
      display.print("Date: ");
      display.print(today);

      unsigned int Sec = timeClient.getSeconds();
      unsigned int Min = timeClient.getMinutes();
      unsigned int Hr = timeClient.getHours();
      if(strMinute.length() == 1)
      {
        strMinute = "0" + strMinute;
      }
      bool PMstatus = false;
      if(Hr >= 12)
      {
        PMstatus = true;
        if(Hr != 12)
          Hr -= 12;
      }
      String strMinute = String(Min);
      String strHour = String(Hr);
      if(PMstatus)
      { 
        current_time = strHour + ":" + strMinute + " PM";
      }
      else
      {
        current_time = strHour + ":" + strMinute + " AM";
      }           
      display.setCursor(0, 20);
      display.print("Time: "); 
      display.print(current_time);

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
    }
    
    display.display();//to actually display the text
}
