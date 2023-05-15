//240 x 320
//https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
//http://adafruit.github.io/Adafruit-GFX-Library/html/class_adafruit___g_f_x.html
//http://www.rinkydinkelectronics.com/calc_rgb565.php
//https://www.reddit.com/r/arduino/comments/khmc75/discord_muted_led_notification_with_esp8266_and/

//Spotify
//https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/bearssl-client-secure-class.html#mfln-or-maximum-fragment-length-negotiation-saving-ram 
//https://github.com/witnessmenow/spotify-api-arduino/blob/main/examples/getCurrentlyPlaying/getCurrentlyPlaying.ino
//https://github.com/spotify/web-api/issues/717
//https://github.com/daspartho/SpotiByeAds

#include <Ticker.h>
#include <time.h>
#include <simpleDSTadjust.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#define SPOTIFY_MARKET "IE"
#define SPOTIFY_REFRESH_TOKEN "AQB94Z_zke9qIK9cH3m0P92Mh3iPuwqxU37RsKZ1efqpP-aGNH0HpAldBXhQwnl2TXvstMXfLSpAS_ESGPorZYnw29CnmGrjH3vRlRJXMPg7zqOBlOMAo5M9_B0o7ykMrhY"


#include "SpotifyArduino.h"
#include "SpotifyArduinoCert.h"
#include <ArduinoJson.h>

//DST adjustment in updateTime2
//check config.h if discord is not working
//if spotify is not working, run getRefreshToken https://github.com/witnessmenow/spotify-api-arduino/tree/main/examples/getRefreshToken
//also double check SPOTIFY_FINGERPRINT is correct for spotify.com (https://www.grc.com/fingerprints.htm)

int timezone = -8;
struct dstRule StartRule PROGMEM = {"PDT", Second, Sun, Mar, 2, 3600}; // Pacific Daylight time = UTC/GMT -7 hours
struct dstRule EndRule PROGMEM = {"PST", First, Sun, Nov, 1, 0};       // Pacific Standard time = UTC/GMT -8 hours
simpleDSTadjust dstAdjusted(StartRule, EndRule);
Ticker ticker1;
int32_t tick;



#define NTP_UPDATE_INTERVAL_SEC 5*3600
#define NTP_SERVERS "us.pool.ntp.org", "pool.ntp.org", "time.nist.gov"

class User {
  public:
    String username;
    boolean muteStat;
    boolean deafStat;
    boolean callStat;
    String callId;

    bool operator<(const User &a) const
    {
      return (username < a.username);
    }

};
boolean topBox = false;
boolean isInCall = false;
int globalHour = 0;
int globalMinute = 0;
String globalDayOfWeek = "";
int globalSeconds = 0;
String globalFormatted = "";
//boolean On = true;

String masterId;
time_t startTime;
time_t spotifyAdStartTime;
time_t spotifyStartTime;
time_t lastSpotifyTime;

int prevX = 0;
int prevY = 0;
int prevX2 = 0;
int prevY2 = 0;
boolean anyoneInCall = false;
//RGB 565
#define BLACK    0x0000//0x31A6//0x0000
#define BLUE     0x19AC //0x0290 //0x2AAE
#define RED      0xF800
#define GREEN    0x07E0 
#define SPOTIFYGREEN 0x1DCA
#define WHITE    0xFFFF
#define LIGHT_GRAY  0xD69A
#define GRAY 0x4208
//#define CYAN     0x07FF
//#define YELLOW   0xFFE0

#include "icons.c"
#include "icons2.c"
//#include <iostream>
#include "config.h"

#include <HardwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include "WebSocketClient.h"
//#include "libs/ArduinoJson.h"

#define DEBUG_APP
#ifdef DEBUG_APP
#define DEBUG_MSG Serial.println
#else
#define DEBUG_MSG(MSG)
#endif


boolean clockScreen = true;
boolean spotifyScreen = false;
int drawWidth = 0;
//ili9341
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#ifdef ESP8266
#define STMPE_CS 16
#define TFT_CS   0
#define TFT_DC   15
#define SD_CS    2
#elif defined(ESP32)
#define STMPE_CS 32
#define TFT_CS   15
#define TFT_DC   33
#define SD_CS    14
#elif defined(TEENSYDUINO)
#define TFT_DC   10
#define TFT_CS   4
#define STMPE_CS 3
#define SD_CS    8
#elif defined(ARDUINO_STM32_FEATHER)
#define TFT_DC   PB4
#define TFT_CS   PA15
#define STMPE_CS PC7
#define SD_CS    PC5
#elif defined(ARDUINO_NRF52832_FEATHER)  /* BSP 0.6.5 and higher! */
#define TFT_DC   11
#define TFT_CS   31
#define STMPE_CS 30
#define SD_CS    27
#elif defined(ARDUINO_MAX32620FTHR) || defined(ARDUINO_MAX32630FTHR)
#define TFT_DC   P5_4
#define TFT_CS   P5_3
#define STMPE_CS P3_3
#define SD_CS    P3_2
#else
// Anything else, defaults!
#define STMPE_CS 6
#define TFT_CS   9
#define TFT_DC   10
#define SD_CS    5
#endif

//#include <Fonts/FreeSans9pt7b.h>

//time
#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org");//"pool.ntp.org");
//Week Days
const String weekDays[7] PROGMEM = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
const String months[12] PROGMEM = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
void setup_wifi();

WiFiClientSecure client;
SpotifyArduino spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);
WiFiClientSecure client2;
WebSocketClient ws2(client2);
DynamicJsonDocument doc(1024);

//const char *host = "discord.com";
//const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80

unsigned long heartbeatInterval = 0;
unsigned long lastHeartbeatAck = 0;
unsigned long lastHeartbeatSend = 0;

unsigned long delayBetweenRequests = 000; // Time between requests 
unsigned long requestDueTime;               //time when request due
boolean isPlaying;
String artist;
String song;
String prevSong = "`";
long duration;
long progress;


bool hasWsSession = false;
String websocketSessionId;
bool hasReceivedWSSequence = false;
unsigned long lastWebsocketSequence = 0;
int holdPin = 0;
#include <list>
std::list<User> inCall;

//#include <Adafruit_STMPE610.h>
//Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);



void setup()
{
  Serial.begin(115200);
  tft.begin();
  //pinMode(LED_PIN, OUTPUT);
  //digitalWrite(LED_PIN, LOW);

  setup_wifi();

  timeClient.begin();

  timeClient.setTimeOffset(-7 * 60 * 60);

  tft.fillScreen(BLACK);
  //tft.fillRect(0, 0, 240, 60, BLACK);
  topBox = false;
  //updateTime2(anyoneInCall);
  timeClient.update();

  updateNTP();
  tick = NTP_UPDATE_INTERVAL_SEC; // Init the NTP update countdown ticker
  ticker1.attach(1, secTicker); // Run a 1 second interval Ticker

  client.setBufferSizes(4096, 4096);
  client2.setBufferSizes(2048, 2048);
  
  client.setFingerprint(SPOTIFY_FINGERPRINT);
  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
    Serial.println("Failed to get access tokens");
  }
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  //Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


}

void printCurrentlyPlayingToSerial(CurrentlyPlaying currentlyPlaying)
{
  // Use the details in this method or if you want to store them
  // make sure you copy them (using something like strncpy)
  // const char* artist =

  //Serial.println("--------- Currently Playing ---------");

  //Serial.print("Is Playing: ");
  if (isPlaying == false && currentlyPlaying.isPlaying == true)
  {
    if (difftime(time(0), lastSpotifyTime) > 300)
    {
      spotifyStartTime = time(0);
    }
  }
  if (currentlyPlaying.isPlaying)
  {
    //Serial.println("Yes");
    isPlaying = true;
    lastSpotifyTime = time(0);
  }
  else
  {
    //Serial.println("No");
    isPlaying = false;
  }

  //Serial.print("Track: ");
  //Serial.println(currentlyPlaying.trackName);
  song = currentlyPlaying.trackName;
  if(song.indexOf("(") > 0)
  {
    song = song.substring(0, song.indexOf("("));
  }
  //Serial.print("Track URI: ");
  //Serial.println(currentlyPlaying.trackUri);
  //Serial.println();
  /*
  Serial.println("Artists: ");
  for (int i = 0; i < currentlyPlaying.numArtists; i++)
  {
    Serial.print("Name: ");
    Serial.println(currentlyPlaying.artists[i].artistName);
    artist = currentlyPlaying.artists[i].artistName;
    Serial.print("Artist URI: ");
    Serial.println(currentlyPlaying.artists[i].artistUri);
    Serial.println();
  }
  */
  /*
  Serial.print("Album: ");
  Serial.println(currentlyPlaying.albumName);
  Serial.print("Album URI: ");
  Serial.println(currentlyPlaying.albumUri);
  Serial.println();
  */
  long progress2 = currentlyPlaying.progressMs; // duration passed in the song
  progress = (int)progress2;
  long duration2 = currentlyPlaying.durationMs; // Length of Song
  duration = (int)duration2;
  //Serial.print("Elapsed time of song (ms): ");
  //Serial.print(progress2);
  //Serial.print(" of ");
  //Serial.println(duration2);
  //Serial.println();
  /*
  float percentage = ((float)progress / (float)duration) * 100;
  int clampedPercentage = (int)percentage;
  Serial.print("<");
  for (int j = 0; j < 50; j++)
  {
    if (clampedPercentage >= (j * 2))
    {
      Serial.print("=");
    }
    else
    {
      Serial.print("-");
    }
  }
  Serial.println(">");
  Serial.println();
  /*
  // will be in order of widest to narrowest
  // currentlyPlaying.numImages is the number of images that
  // are stored
  /*
    for (int i = 0; i < currentlyPlaying.numImages; i++)
    {
      Serial.println("------------------------");
      Serial.print("Album Image: ");
      Serial.println(currentlyPlaying.albumImages[i].url);
      Serial.print("Dimensions: ");
      Serial.print(currentlyPlaying.albumImages[i].width);
      Serial.print(" x ");
      Serial.print(currentlyPlaying.albumImages[i].height);
      Serial.println();
    }
    Serial.println("------------------------");
  */
}


void loop()
{
  updateNTP();

  /*
    // Generic HTTPS client for sending messages at some point

    WiFiClientSecure httpsClient;

    Serial.print("HTTPS Connecting");
    int r=0; //retry counter
    while((!httpsClient.connect(host, httpsPort)) && (r < 30))
    {
       delay(100);
       Serial.print(".");
       r++;
    }
    if(r==30)
    {
       Serial.println("Connection failed");
    }
    else
    {
       Serial.println("Connected to web");
    }
    httpsClient.print(String("GET ") + "https://discord.com/api/gateway" + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" +
              "Authorization: " + bot_token + "\r\n" +
              "Connection: close\r\n\r\n");
    Serial.println("request sent");

    while (httpsClient.connected())
    {
       String line = httpsClient.readStringUntil('\n');
       if (line == "\r")
      {
           Serial.println("headers received");
           break;
      }
    }
    Serial.println("reply was:");
    Serial.println("==========");
    String line;
    while(httpsClient.available())
    {
       line = httpsClient.readStringUntil('\n');  //Read Line by Line
       Serial.println(line); //Print response
    }
    Serial.println("==========");
    Serial.println("closing connection");

    delay(2000);  //GET Data at every 2 seconds
  */




  if (!ws2.isConnected())
  {
    Serial.println("connecting");
    ws2.setSecureFingerprint(certificateFingerprint);
    // It technically should fetch url from discord.com/api/gateway
    ws2.connect("gateway.discord.gg", "/?v=9&encoding=json", 443);
  }
  else
  {

    unsigned long now = millis();
    if (heartbeatInterval > 0)
    {
      if (now > lastHeartbeatSend + heartbeatInterval)
      {
        if (hasReceivedWSSequence)
        {
          DEBUG_MSG("Send:: {\"op\":1,\"d\":" + String(lastWebsocketSequence, 10) + "}");
          ws2.send("{\"op\":1,\"d\":" + String(lastWebsocketSequence, 10) + "}");
        }
        else
        {
          DEBUG_MSG("Send:: {\"op\":1,\"d\":null}");
          ws2.send("{\"op\":1,\"d\":null}");
        }
        lastHeartbeatSend = now;
      }
      if (lastHeartbeatAck > lastHeartbeatSend + (heartbeatInterval / 2))
      {
        DEBUG_MSG("Heartbeat ack timeout");
        ws2.disconnect();
        heartbeatInterval = 0;
      }
    }

    String msg;
    if (ws2.getMessage(msg))
    {
      Serial.println(msg);
      deserializeJson(doc, msg);


      // TODO Should maintain heartbeat
      if (doc["op"] == 0) // Message
      {
        if (doc.containsKey("s"))
        {
          lastWebsocketSequence = doc["s"];
          hasReceivedWSSequence = true;
        }

        if (doc["t"] == "READY")
        {
          websocketSessionId = doc["d"]["session_id"].as<String>();
          hasWsSession = true;
        }
        else if (doc["t"] == "GUILD_CREATE")
        {
          for (JsonVariant v : doc["d"]["voice_states"].as<JsonArray>())
          {
            if (v["user_id"] == USER_ID)
            {
              if (v["self_mute"] || v["mute"])
              {
                tft.fillScreen(ILI9341_RED);


              }
              else
              {
                tft.fillScreen(ILI9341_BLACK);
              }


              break;
            }
          }
        }
        else if (doc["t"] == "VOICE_STATE_UPDATE")
        {
          int yPos = 50 + 20;

          //tft.fillRect(0,0,240, 60, BLACK);

          if (doc["d"]["member"]["user"]["id"] == USER_ID)
          {


            if (msg.substring(msg.length() - 6, msg.length() - 6 + 4) != "null")
            {
              tft.fillScreen(BLACK);
              tft.fillRect(0, 60, 240, 320, GRAY);

              tft.fillRect(8, 297, 6, 5, GREEN);
              tft.fillRect(19, 290, 6, 12, GREEN);
              tft.fillRect(30, 282, 6, 20, GREEN);

              tft.setTextColor(GREEN);
              tft.setCursor(46, 286);
              tft.setTextSize(2);
              tft.println("Voice Connected");

              masterId = msg.substring(msg.lastIndexOf(':') + 2, msg.length() - 3);
              Serial.println(masterId);

              if (!isInCall)
              {

                startTime = time(0);
              }

              isInCall = true;
            }
            else
            {
              tft.fillRect(0, 280, 240, 320 - 280, GRAY);
              isInCall = false;
              masterId = "";
              Serial.println("called");
            }
          }


          //update



          for (std::list<User>::iterator it = inCall.begin(); it != inCall.end(); ++it) {
            User aUser = *it;
            if (String(doc["d"]["member"]["user"]["username"]) == aUser.username)
            {

              inCall.erase(it);
              break;
            }

          }



          User thisUser;
          thisUser.username = String(doc["d"]["member"]["user"]["username"]);
          boolean deafStat2 = false;
          boolean muteStat2 = false;
          boolean callStat2 = false;
          thisUser.callId = msg.substring(msg.lastIndexOf(':') + 2, msg.length() - 3);
          if ((doc["d"]["deaf"] || doc["d"]["self_deaf"]))
            deafStat2 = true;
          if ((doc["d"]["self_mute"] || doc["d"]["mute"]))
            muteStat2 = true;

          if (msg.substring(msg.length() - 6, msg.length() - 6 + 4) != "null") //joning call
            callStat2 = true;

          thisUser.muteStat = muteStat2;
          thisUser.deafStat = deafStat2;
          thisUser.callStat = callStat2;

          inCall.push_back(thisUser);

          inCall.sort();

          anyoneInCall = false;
          for (std::list<User>::iterator it = inCall.begin(); it != inCall.end(); ++it) {
            User aUser = *it;
            if (aUser.callStat && aUser.callId == masterId)
            {
              if (!anyoneInCall)
              {
                if (!topBox)
                  tft.fillRect(0, 0, 240, 60, BLACK);

              }
              anyoneInCall = true;
              break;
            }
          }

          if (anyoneInCall)
          {
            tft.fillRect(0, 60, 240, 222, GRAY);
            clockScreen = false;
            spotifyScreen = false;
            for (std::list<User>::iterator it = inCall.begin(); it != inCall.end(); ++it) {
              User aUser = *it;
              if (aUser.callStat && aUser.callId == masterId)
              {

                tft.setCursor(5, yPos + 10);
                tft.setTextColor(ILI9341_WHITE);

                tft.setTextSize(2.5);
                tft.println(aUser.username);

                tft.drawRGBBitmap(142, yPos + 2, Muted, 30, 30);
                tft.drawRGBBitmap(192, yPos + 2, Deafen, 30, 30);

                if (aUser.muteStat)
                {
                  for (int step = 0; step < 30; step += 1)
                  {
                    int itX = 142 + step;
                    int itY = yPos + 30 - step;
                    tft.fillCircle(itX, itY, 1, ILI9341_RED);

                  }
                }
                if (aUser.deafStat)
                {
                  for (int step = 0; step < 30; step += 1)
                  {
                    int itX = 192 + step;
                    int itY = yPos + 30 - step;
                    tft.fillCircle(itX, itY, 1, ILI9341_RED);

                  }
                  for (int step = 0; step < 30; step += 1)
                  {
                    int itX = 142 + step;
                    int itY = yPos + 30 - step;
                    tft.fillCircle(itX, itY, 1, ILI9341_RED);

                  }
                }
                yPos += 40;
              }


            }

          }





        }
      }
      else if (doc["op"] == 9) // Connection invalid
      {
        ws2.disconnect();
        hasWsSession = false;
        heartbeatInterval = 0;
      }
      else if (doc["op"] == 11) // Heartbeat ACK
      {
        lastHeartbeatAck = now;
      }
      else if (doc["op"] == 10) // Start
      {
        heartbeatInterval = doc["d"]["heartbeat_interval"];

        if (hasWsSession)
        {
          String msg = "{\"op\":6,\"d\":{\"token\":\"" + String(bot_token) + "\",\"session_id\":\"" + websocketSessionId + "\",\"seq\":\"" + String(lastWebsocketSequence, 10) + "\"}}";
          DEBUG_MSG("Send:: " + msg);
          ws2.send(msg);
        }
        else
        {
          String msg = "{\"op\":2,\"d\":{\"token\":\"" + String(bot_token) + "\",\"intents\":" + gateway_intents + ",\"properties\":{\"$os\":\"linux\",\"$browser\":\"ESP8266\",\"$device\":\"ESP8266\"},\"compress\":false,\"large_threshold\":250}}";
          DEBUG_MSG("Send:: " + msg);
          ws2.send(msg);
        }

        lastHeartbeatSend = now;
        lastHeartbeatAck = now;
      }
    }

  }


  if (!anyoneInCall)
  {
 
      if (millis() > requestDueTime)
      {
      //Serial.println("----------Transitioning to Spotify------------------");
      //Serial.print("Free Heap: ");
      //Serial.println(ESP.getFreeHeap());

      Serial.println("getting currently playing song:");
      // Market can be excluded if you want e.g. spotify.getCurrentlyPlaying()
      int status = spotify.getCurrentlyPlaying(printCurrentlyPlayingToSerial);//, SPOTIFY_MARKET);
      if (status == 200)
      {
        Serial.println("Successful ly got currently playing");
       
      }
      else if (status == 204)
      {
        Serial.println("Doesn't seem to be anything playing");
        isPlaying = false;
      }
      else
      {
        Serial.print("Error: ");
        Serial.println(status);
        Serial.println("Refreshing Access Tokens");
        if (!spotify.refreshAccessToken())
        {
          Serial.println("Failed to get access tokens");
        }
      }
        requestDueTime = millis() + delayBetweenRequests;
        //Serial.println("----------Leaving Spotify------------------");
      }
  
  
  }



  updateTime2();


  if (anyoneInCall)
  {

    drawBoxClock();
  }
  else if (isPlaying)
  {
    
    drawSpotifyClock();
  }
  else
  {
    drawCircleClock();
  }

}

void updateTime2()
{
  timeClient.update();




  globalHour = ((timeClient.getHours() + 10) + 1) % 12 + 1; //////Daylight Savings Time
  //globalHour = ((timeClient.getHours() + 10) + 1) % 12;
  if (globalHour == 0)
  {
    globalHour = 12;
  }
  globalMinute = timeClient.getMinutes();
  globalDayOfWeek = String(weekDays[timeClient.getDay()]);
  globalSeconds = timeClient.getSeconds();
  globalFormatted = timeClient.getFormattedTime();

}

void drawBoxClock()
{
  tft.setCursor(42, 8);
  tft.setTextColor(ILI9341_WHITE, BLACK);
  tft.setTextSize(3);
  /*


  */
  int second = difftime( time(0), startTime);
  int minute = (second / 60);
  int hour = minute / 60;
  second = second % 60;
  minute = minute % 60;

  String hours = String(hour);
  if (hours.length() < 2)
    hours = "0" + hours;
  String minutes =  String(minute);
  if (minutes.length() < 2)
    minutes = "0" + minutes;
  String seconds =  String(second);
  if (seconds.length() < 2)
    seconds = "0" + seconds;
  /*
    String formattedTime = String(globalHour + globalFormatted.substring(globalFormatted.indexOf(":"), 10));
    if (formattedTime.length() == 7)
    formattedTime = "0" + formattedTime;
  */
  tft.setTextColor(ILI9341_WHITE, BLACK);
  tft.println(String(hours) + ":" + String(minutes) + ":" + String(seconds));


  uint16_t w, h;
  int16_t x1, y1;
  tft.setTextSize(2);
  tft.getTextBounds(globalDayOfWeek.substring(0, 3) + ", " + getDate(0), 0, 0, &x1, &y1, &w, &h); //calc width of new string


  tft.fillRect(0, 0, (240 - w) / 2, 60, BLACK);
  tft.fillRect(240 - (240 - w) / 2, 0, 240, 60, BLACK);
  tft.setCursor((240 - w) / 2, 36);
  tft.setTextColor(ILI9341_WHITE, BLACK);
  tft.println(globalDayOfWeek.substring(0, 3) + ", " + getDate(0));
  topBox = true;
  clockScreen = false;
  spotifyScreen = false;
  isPlaying = false;

}

void drawCircleClock()
{
  //timeClient.update();

  if (!clockScreen)
  {
    tft.fillScreen(BLACK);
    clockScreen = true;
  }

  topBox = false;
  spotifyScreen = false;



  //unsigned long epochTime = timeClient.getEpochTime();
  //struct tm *ptm = gmtime ((time_t *)&epochTime);
  //int currentMonth = ptm->tm_mon+1;


  //int monthDay = ptm->tm_mday;

  uint16_t w, h;
  int16_t x1, y1;


  tft.setTextSize(3);

  String minutes2 = String(globalMinute);
  if (minutes2.length() != 2)
    minutes2 = "0" + minutes2;
  tft.getTextBounds("  " + String(globalHour) + ":" + minutes2 + "  ", 0, 0, &x1, &y1, &w, &h); //calc width of new string
  tft.setTextColor(WHITE, BLACK);
  tft.setCursor((240 - w) / 2, 120);
  tft.println("  " + String(globalHour) + ":" + minutes2 + "  ");


  tft.setTextSize(2);
  tft.getTextBounds("  " + getDate(0) + "  ", 0, 0, &x1, &y1, &w, &h); //calc width of new string

  tft.setCursor((240 - w) / 2, 148);
  tft.println("  " + getDate(0) + "  ");

  tft.getTextBounds("  " + globalDayOfWeek + "  ", 0, 0, &x1, &y1, &w, &h); //calc width of new string
  tft.setCursor((240 - w) / 2, 174);
  tft.println("  " + globalDayOfWeek + "  ");


  tft.fillCircle(prevX, prevY, 4, BLACK);
  int angle = round(((globalHour) * 60 + globalMinute) / 2);
  float rad = angle * PI / 180;
  int x = int(120 + 110 * sin(rad));
  int y = abs(-160 + 110 * cos(rad));
  tft.fillCircle(x, y, 4, WHITE);
  tft.drawCircle (240 / 2, 320 / 2, 110, WHITE);

  prevX = x;
  prevY = y;


  tft.fillCircle(prevX2, prevY2, 6, BLACK);
  int angle2 = round(globalMinute * 6);
  float rad2 = angle2 * PI / 180;
  int x2 = int(120 + 110 * sin(rad2));
  int y2 = abs(-160 + 110 * cos(rad2));
  tft.fillCircle(x2, y2, 6, WHITE);
  tft.drawCircle (240 / 2, 320 / 2, 110, WHITE);

  prevX2 = x2;
  prevY2 = y2;
}

void drawSpotifyClock() // "pill" version
{
  //timeClient.update();
  int offset = -30;
  uint16_t w, h;
  int16_t x1, y1;

  if (!spotifyScreen)
  {
    tft.fillScreen(BLACK);
    //tft.fillRect(5, 320 - 60, 240-10, 60, GRAY);
    tft.fillRoundRect(5, 320 - 60, 240-10, 60, 15, GRAY);
    spotifyScreen = true;
    prevSong = "`";
    //total time round rectangle
    tft.setTextSize(2);
    tft.getTextBounds("0:00", 0, 0, &x1, &y1, &w, &h); //calc width of new string
    tft.fillRoundRect(120-(w+40)/2, 186- 9, w+40, h+16, 15, GRAY);
  }

  topBox = false;
  clockScreen = false;


  //unsigned long epochTime = timeClient.getEpochTime();
  //struct tm *ptm = gmtime ((time_t *)&epochTime);
  //int currentMonth = ptm->tm_mon+1;


  //int monthDay = ptm->tm_mday;

  

  tft.setTextSize(3);

  String minutes2 = String(globalMinute);
  if (minutes2.length() != 2)
    minutes2 = "0" + minutes2;
  tft.getTextBounds("  " + String(globalHour) + ":" + minutes2 + "  ", 0, 0, &x1, &y1, &w, &h); //calc width of new string
  tft.setTextColor(WHITE, BLACK);
  tft.setCursor((240 - w) / 2, 120 + offset);
  tft.println("  " + String(globalHour) + ":" + minutes2 + "  ");


  tft.setTextSize(2);
  tft.getTextBounds("  " + getDate(0) + "  ", 0, 0, &x1, &y1, &w, &h); //calc width of new string

  tft.setCursor((240 - w) / 2, 148 + offset);
  tft.println("  " + getDate(0) + "  ");

  tft.getTextBounds("  " + globalDayOfWeek + "  ", 0, 0, &x1, &y1, &w, &h); //calc width of new string
  tft.setCursor((240 - w) / 2, 174 + offset);
  tft.println("  " + globalDayOfWeek + "  ");


  tft.fillCircle(prevX, prevY + offset, 4, BLACK);
  int angle = round(((globalHour) * 60 + globalMinute) / 2);
  float rad = angle * PI / 180;
  int x = int(120 + 110 * sin(rad));
  int y = abs(-160 + 110 * cos(rad));
  tft.fillCircle(x, y + offset, 4, WHITE);
  //tft.drawCircle (240 / 2, 320 / 2 + offset, 110, WHITE);

  prevX = x;
  prevY = y;


  tft.fillCircle(prevX2, prevY2 + offset, 6, BLACK);
  int angle2 = round(globalMinute * 6);
  float rad2 = angle2 * PI / 180;
  int x2 = int(120 + 110 * sin(rad2));
  int y2 = abs(-160 + 110 * cos(rad2));
  tft.fillCircle(x2, y2 + offset, 6, WHITE);
  tft.drawCircle (240 / 2, 320 / 2 + offset, 110, WHITE);

  prevX2 = x2;
  prevY2 = y2;

  
  int seconds = difftime( time(0), spotifyStartTime);
  int minutes = seconds / 60;
  int hours = minutes/60;
  //seconds = seconds % 60;
  minutes = minutes % 60;
  tft.setTextSize(2);
  String text;
  if (minutes < 10)
  {
    text = String(hours) + ":0" + String(minutes);
  }
  else
  {
    text=String(hours) + ":" + String(minutes);
  }
  globalSeconds = seconds;
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h); //calc width of new string
  //tft.fillRoundRect(120-(w+40)/2, 184- 9, w+40, h+16, 15, GRAY);
  tft.setCursor(240/2-w/2, 186);
  tft.setTextColor(WHITE,GRAY);
  tft.println(text);

  //spotify info
  tft.setTextColor(WHITE, GRAY);
  if (song != "")
  {
    delayBetweenRequests = 0;
    if (song != prevSong)
    {
      tft.fillRoundRect(5, 320 - 60, 240-10, 60, 15, GRAY);
      tft.getTextBounds(song, 0, 0, &x1, &y1, &w, &h); //calc width of new string
      int size = 3;
      tft.setTextWrap(false); 
      while (w > 240-15)
      {
        size = size-0.05;
        tft.setTextSize(size);
        tft.getTextBounds(song, 0, 0, &x1, &y1, &w, &h); //calc width of new string
      }
      
      tft.setCursor((240 - w) / 2, 320 - 51);
      drawWidth = 0;
      tft.println(song);
      prevSong = song;
    }
    tft.fillCircle(drawWidth+20, 320-27, 5, GRAY);
    tft.fillRect(20, 320-27, 240 - 40, 3, LIGHT_GRAY);
    float percentage = ((float)progress / (float)duration);
    drawWidth = round((240-40)*percentage);
    tft.fillRect(20, 320-27, drawWidth, 3, GREEN);
    tft.fillCircle(drawWidth+20, 320-27, 5, GREEN);
  
    tft.setCursor(15, 320-17);
    tft.setTextSize(1.5);
    int seconds = progress/1000;
    int minutes = seconds/60;
    seconds = seconds%60;
    if (seconds < 10)
    {
      tft.println(String(minutes) + ":0" + String(seconds));
    }
    else
    {
      tft.println(String(minutes) + ":" + String(seconds));
    }
    
  
    seconds = duration/1000;
    minutes = seconds/60;
    seconds = seconds%60;
    tft.getTextBounds(String(minutes) + ":" + String(seconds), 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(240-15-w, 320-17);
    if (seconds < 10)
    {
      tft.println(String(minutes) + ":0" + String(seconds));
    }
    else
    {
      tft.println(String(minutes) + ":" + String(seconds));
    }

  }
  else
  {
    delayBetweenRequests = 5000;
    String text;
    if (prevSong != "")
    {
      //start timer
      tft.fillRoundRect(5, 320 - 60, 240-10, 60, 15, GRAY);
      spotifyAdStartTime = time(0);
      tft.setTextSize(2);
      tft.getTextBounds("0:00", 0, 0, &x1, &y1, &w, &h); //calc width of new string
      tft.setCursor(240/2-w/2, 320-25);
      tft.println("0:00");
    }
    else
    {
      tft.setTextSize(1.5);
      int seconds = difftime( time(0), spotifyAdStartTime);
      int minute = (seconds / 60);
      seconds = seconds % 60;
      minute = minute % 60;
      tft.setTextSize(2);
      
     
      if (seconds < 10)
      {
        text = String(minute) + ":0" + String(seconds);
      }
      else
      {
        text=String(minute) + ":" + String(seconds);
      }
      globalSeconds = seconds;
      tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h); //calc width of new string
      tft.setCursor(240/2-w/2, 320-25);
      tft.println(text);
    }
    
    tft.setTextSize(2.5);
    tft.getTextBounds("Ad Playing", 0, 0, &x1, &y1, &w, &h); //calc width of new string
    tft.setCursor(240/2-60, 320-50);
    tft.println("Ad Playing");
    prevSong = "";
  }
}
/*
void drawSpotifyClock()
{
  //timeClient.update();
  int offset = -30;
  if (!spotifyScreen)
  {
    tft.fillScreen(BLACK);
    //tft.fillRect(5, 320 - 60, 240-10, 60, GRAY);
    tft.fillRoundRect(5, 320 - 60, 240-10, 60, 15, SPOTIFYGREEN);
    spotifyScreen = true;
    prevSong = "";
  }

  topBox = false;
  clockScreen = false;


  //unsigned long epochTime = timeClient.getEpochTime();
  //struct tm *ptm = gmtime ((time_t *)&epochTime);
  //int currentMonth = ptm->tm_mon+1;


  //int monthDay = ptm->tm_mday;

  uint16_t w, h;
  int16_t x1, y1;


  tft.setTextSize(3);

  String minutes2 = String(globalMinute);
  if (minutes2.length() != 2)
    minutes2 = "0" + minutes2;
  tft.getTextBounds("  " + String(globalHour) + ":" + minutes2 + "  ", 0, 0, &x1, &y1, &w, &h); //calc width of new string
  tft.setTextColor(WHITE, BLACK);
  tft.setCursor((240 - w) / 2, 120 + offset);
  tft.println("  " + String(globalHour) + ":" + minutes2 + "  ");


  tft.setTextSize(2);
  tft.getTextBounds("  " + getDate(0) + "  ", 0, 0, &x1, &y1, &w, &h); //calc width of new string

  tft.setCursor((240 - w) / 2, 148 + offset);
  tft.println("  " + getDate(0) + "  ");

  tft.getTextBounds("  " + globalDayOfWeek + "  ", 0, 0, &x1, &y1, &w, &h); //calc width of new string
  tft.setCursor((240 - w) / 2, 174 + offset);
  tft.println("  " + globalDayOfWeek + "  ");


  tft.fillCircle(prevX, prevY + offset, 4, BLACK);
  int angle = round(((globalHour) * 60 + globalMinute) / 2);
  float rad = angle * PI / 180;
  int x = int(120 + 110 * sin(rad));
  int y = abs(-160 + 110 * cos(rad));
  tft.fillCircle(x, y + offset, 4, WHITE);
  tft.drawCircle (240 / 2, 320 / 2 + offset, 110, WHITE);

  prevX = x;
  prevY = y;


  tft.fillCircle(prevX2, prevY2 + offset, 6, BLACK);
  int angle2 = round(globalMinute * 6);
  float rad2 = angle2 * PI / 180;
  int x2 = int(120 + 110 * sin(rad2));
  int y2 = abs(-160 + 110 * cos(rad2));
  tft.fillCircle(x2, y2 + offset, 6, WHITE);
  tft.drawCircle (240 / 2, 320 / 2 + offset, 110, WHITE);

  prevX2 = x2;
  prevY2 = y2;


  //spotify info
  
  if (song != prevSong)
  {
    //tft.fillRect(0, 320 - 60, 240, 60, GRAY);
    tft.fillRoundRect(5, 320 - 60, 240-10, 60, 15, SPOTIFYGREEN);
    tft.getTextBounds(song, 0, 0, &x1, &y1, &w, &h); //calc width of new string
    tft.setTextColor(WHITE, BLACK);
    int size = 3;
    tft.setTextWrap(false); 
    while (w > 240)
    {
      size = size-0.1;
      tft.setTextSize(size);
      tft.getTextBounds(song, 0, 0, &x1, &y1, &w, &h); //calc width of new string
    }
    
    tft.setCursor((240 - w) / 2, 320 - 50);
    drawWidth = 0;
    tft.println(song);
    prevSong = song;
  }
  tft.fillCircle(drawWidth+15, 320-22, 5, BLACK);
  tft.fillRect(15, 320-22, 240 - 30, 2, WHITE);
  float percentage = ((float)progress / (float)duration);
  Serial.print("Progress: ");
  Serial.println(progress);
  Serial.print("Duration: ");
  Serial.println(duration);
  Serial.println(percentage);
  drawWidth = round((240-30)*percentage);
  tft.fillRect(15, 320-22, drawWidth, 2, GREEN);
  tft.fillCircle(drawWidth+15, 320-22, 5, GREEN);

  tft.setCursor(7, 320-15);
  tft.setTextSize(1.5);
  int seconds = progress/1000;
  int minutes = seconds/60;
  seconds = seconds%60;
  if (seconds < 10)
  {
    tft.println(String(minutes) + ":0" + String(seconds));
  }
  else
  {
    tft.println(String(minutes) + ":" + String(seconds));
  }
  

  seconds = duration/1000;
  minutes = seconds/60;
  seconds = seconds%60;
  tft.getTextBounds(String(minutes) + ":" + String(seconds), 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(240-7-w, 320-15);
  if (seconds < 10)
  {
    tft.println(String(minutes) + ":0" + String(seconds));
  }
  else
  {
    tft.println(String(minutes) + ":" + String(seconds));
  }
}

*/
// NTP timer update ticker
void secTicker()
{
  tick--;
  if (tick <= 0)
  {

    tick = NTP_UPDATE_INTERVAL_SEC; // Re-arm
  }

  // printTime(0);  // Uncomment if you want to see time printed every second
}


void updateNTP() {

  configTime(timezone * 3600, 0, NTP_SERVERS);

  //delay(500);
  while (!time(nullptr)) {
    Serial.print("#");
    delay(1000);
  }
}

/*
void printTime(time_t offset)
{
  char buf[30];
  char *dstAbbrev;
  time_t t = dstAdjusted.time(&dstAbbrev) + offset;
  struct tm *timeinfo = localtime (&t);

  int hour = (timeinfo->tm_hour + 11) % 12 + 1; // take care of noon and midnight
  sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d%s %s\n", timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_year + 1900, hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_hour >= 12 ? "pm" : "am", dstAbbrev);
  Serial.print(buf);
}
*/
String getDate(time_t offset)
{
  char buf[30];
  char *dstAbbrev;
  time_t t = dstAdjusted.time(&dstAbbrev) + offset;
  struct tm *timeinfo = localtime (&t);

  int monthNum = timeinfo->tm_mon;
  String month = months[monthNum];
  char date[2];
  sprintf(date, "%02d", timeinfo->tm_mday);
  //Serial.println(month + " " + date);
  return month + " " + date;
}
