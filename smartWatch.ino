#include<DHTesp.h>
#include<TinyGPS++.h>
#include<ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>//Graphics library
#include <Adafruit_SSD1306.h>//Library to control driver chip(SSD1306) in OLED display

DHTesp dht;
TinyGPSPlus gps;

#define DHT11_Pin 2
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET LED_BUILTIN // Reset pin #, not required
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);//making 'display' object. We don't need OLED_RESET but have to define it to declare the object 

char *ssid = "FCTP";
char *pswd = "W!F!_k@_p@$$w0rd";
const char *api_key = "UDWGW2AFE8HCARQ8";
unsigned long int myChannelNumber = 812997;

WiFiClient client;

static int uploadDelay = 0;
String today;
String currentTime;
float Speed;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
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
  
  ThingSpeak.begin(client);//ThingSpeak is server. NodeMCU sending post request through HTTP.
  
  display.clearDisplay();//clear the display
  display.setTextColor(WHITE);//always use WHITE irrespective of actual color display
  display.setTextSize(0.5);//font size
}

void loop()
{
    float temp;
    float hum;
    if(uploadDelay >= 1200)
    {
      temp = dht.getHumidity();
      hum = dht.getTemperature();
      ThingSpeak.setField(1, temp);//int setField(unsigned int field, int value)
      ThingSpeak.setField(2, hum);
      ThingSpeak.writeFields(myChannelNumber, api_key);//int writeField(unsigned long channelNumber, unsigned int field, int value, const char * writeAPIKey)
      uploadDelay = 0; 
    }
    display.setCursor(0, 10);
    display.print("Humidity:   ");
    display.print(hum);
    display.print("%");
    display.setCursor(0, 20);
    display.print("Temperature:   ");
    display.print(temp);
    display.print(" ºC");
    
    uploadDelay += 200;
    delay(200);
    
    if(Serial.available() > 0)
    {
      gps.encode(Serial.read());//have to repeatedly funnel the characters to TinyGPS++ from the GPS module using the encode() method to make it work.
      
      if(gps.location.isValid())//data goes to Google Maps API
      {
        if(gps.location.isUpdated())
        {
          double latitude = gps.location.lat();
          double longitude = gps.location.lng();
          ThingSpeak.setField(4, float(latitude));
          ThingSpeak.setField(5, float(longitude));
          ThingSpeak.writeFields(myChannelNumber, api_key);
        }
      }
      
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
      display.print(today);

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
          String currentTime = strHour + ":" + strMinute + " PM";
        }
        else
        {
          currentTime = strHour + ":" + strMinute + " AM";
        }
      }
      display.setCursor(10, 0);
      display.print(currentTime);
      
      if(gps.speed.isValid())//data goes to thingspeak.com
      {
        Speed = gps.speed.mps();
        ThingSpeak.setField(3, Speed);
        ThingSpeak.writeFields(myChannelNumber, api_key);
      }
    }
    String strSpeed = String(Speed, 2); 
    display.setCursor(0, 30);
    display.print("Speed:  ");
    display.print(strSpeed);
    display.print("m/s");
    
    display.display();//to actually display the text
}
