/**The MIT License (MIT)

Copyright (c) 2016 by Guillaume Bru

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

This software is a fork from the fantastic weather station of Daniel Eichorn http://blog.squix.ch
*/

#include <Wire.h>
#include <Ticker.h>
#include "ssd1306_i2c.h"
#include "icons.h"

#include "TSL2561.h"

#include <ESP8266WiFi.h>

#include <PubSubClient.h>

#include <DHT.h>
#define DHTTYPE DHT22
#define DHTPIN  2

// The address will be different depending on whether you let
// the ADDR pin float (addr 0x39), or tie it to ground or vcc. In those cases
// use TSL2561_ADDR_LOW (0x29) or TSL2561_ADDR_HIGH (0x49) respectively
TSL2561 tsl(TSL2561_ADDR_FLOAT);


// Initialize the oled display for address 0x3c
// sda-pin=14 and sdc-pin=12
SSD1306 display(0x3c, 4, 5);

Ticker ticker;

// Global Setting
// your network SSID (name)
char ssid[] = "your_ssid";
// your network password
char pass[] = "password"; 


// MQTT broker server
const char* mqtt_server = "broker.hivemq.com";

// Client name, use any name you want
const char* clientID = "wimio1";

const char* outTopic =  "wimio1/outTopic";
const char* outTopic1 = "wimio1/Temperature";
const char* outTopic2 = "wimio1/Humidity";
const char* outTopic3 = "wimio1/Luminosity";
const char* inTopic =   "wimio1/inTopic";

String apiKey = "XXXXXXXXXXXXXXXX";
const char* tsserver = "api.thingspeak.com";

// Value read
float humidity, temp_c;  // Values read from sensor
uint16_t lux;

//string converted ..
char chartemp[10];
char charlux[10];
char charhum[10];

// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor

DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266

// flag changed in the ticker function every 10 minutes
bool readyForsenddata = true;

// Use celsius by default, set 0 for fahrenheit
#define UNIT_CELSIUS 1

void drawFrame1(int x, int y); //Temperature
void drawFrame2(int x, int y); //Humidity
void drawFrame3(int x, int y); //Luminosity

void drawSpinner(int count, int active);

// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
void (*frameCallbacks[3])(int x, int y) = {drawFrame1, drawFrame2, drawFrame3};

// how many frames are there?
int frameCount = 3;
// on frame is currently displayed
int currentFrame = 0;
 
WiFiClient espClient;
PubSubClient client(espClient);

char msg[50];


void setreadyForsenddata() {
  readyForsenddata = true;  
}


void getDHT22() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor
    previousMillis = currentMillis;   

    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity = dht.readHumidity();          // Read humidity (percent)
    temp_c = dht.readTemperature();     // Read temperature as Celcius
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temp_c)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
 
    dtostrf(temp_c, 2,1, chartemp);
    dtostrf(humidity, 2,1, charhum);
    
  }
}

void getlux() {

    // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
    // That way you can do whatever math and comparisons you want!
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    lux = tsl.calculateLux(full, ir);

    sprintf(charlux, "%d",lux);   
  
}


void callback(char* topic, byte* payload, unsigned int length) {
  // Conver the incoming byte array to a string
  payload[length] = '\0'; // Null terminator used to terminate the char array
  String message = (char*)payload;

  //topic[length] = '\0'; // Null terminator used to terminate the char array
  Serial.print("Message arrived on topic: [");
  Serial.print(topic);
  Serial.print("], ");
  Serial.println(message);

  if(message == "temperature, c"){
    getDHT22();
    Serial.print("Sending temperature:");
    Serial.println(temp_c);
    dtostrf(temp_c , 2, 2, msg);
    client.publish(outTopic1, msg);
  } else if (message == "humidity"){
    getDHT22();
    Serial.print("Sending humidity:");
    Serial.println(humidity);
    dtostrf(humidity , 2, 2, msg);
    client.publish(outTopic2, msg);
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, clientID);
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void send_data()
{

    getDHT22();
    Serial.print("Sending temperature:");
    Serial.println(temp_c);
    //dtostrf(temp_c , 2, 2, msg);
    client.publish(outTopic1, chartemp);
    Serial.print("Sending humidity:");
    Serial.println(humidity);
    //dtostrf(humidity , 2, 2, msg);
    client.publish(outTopic2, charhum);
    getlux();
    Serial.print("Sending Luminosity:");
    Serial.println(lux);
    client.publish(outTopic3, charlux);

    if (espClient.connect(tsserver,80)) {  //   "184.106.153.149" or api.thingspeak.com
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += chartemp;
           postStr +="&field2=";
           postStr += charhum;
           postStr +="&field3=";
           postStr += charlux;
           postStr += "\r\n\r\n";
 
     espClient.print("POST /update HTTP/1.1\n"); 
     espClient.print("Host: api.thingspeak.com\n"); 
     espClient.print("Connection: close\n"); 
     espClient.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n"); 
     espClient.print("Content-Type: application/x-www-form-urlencoded\n"); 
     espClient.print("Content-Length: "); 
     espClient.print(postStr.length()); 
     espClient.print("\n\n"); 
     espClient.print(postStr);
    }  
    espClient.stop();
}

void setup() {
  // initialize dispaly
  display.init();
  // For some users
  display.flipScreenVertically();
  // set the drawing functions
  display.setFrameCallbacks(3, frameCallbacks);
  // how many ticks does a slide of frame take?
  display.setFrameTransitionTicks(60);

  display.clear();
  display.display();

  if (tsl.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No sensor?");
    while (1);
  }

  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2561_GAIN_0X);         // set no gain (for bright situtations)
  tsl.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)
  
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
  //tsl.setTiming(TSL2561_INTEGRATIONTIME_101MS);  // medium integration time (medium light)
  //tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS);  // longest integration time (dim light)
  
  
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  
  int counter = 0;
  int heartbeat=1;
  int heartspeed=+40;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    display.clear();
    display.drawXbm(34, 10, 60, 36, WiFi_Logo_bits);
    //display.setColor(INVERSE);
    //display.fillRect(10, 10, 108, 44);
    //display.setColor(WHITE);
    display.setContrast(heartbeat%256);
    heartbeat+=heartspeed;
    if(heartbeat > 255){
      heartspeed*=-1;
      heartbeat=255;
      }
    else if(heartbeat <1){
      heartspeed*=-1;
      heartbeat=1;
    }
    display.drawString(27, 50 , "Connecting" );

    //drawSpinner(3, counter % 3);
    display.display();
    delay(5);
    counter++;
  }
  
  display.setContrast(0);//Power Save and keep oleds from burning.

  Serial.println("");
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("add mqtt");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();           // initialize temperature sensor

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
  if (readyForsenddata) {
        readyForsenddata = false;
        send_data();  
  }
  display.clear();
  display.nextFrameTick();
  display.display();
  client.loop();

}


String unitConv( String fahrenheit ){
  float fvar = fahrenheit.toFloat();
  int val = fvar;
  if( UNIT_CELSIUS )
    val = (fvar-32)/1.8 ; //that deletes floating point
  String ret(val);
  ret+= UNIT_CELSIUS ? "C":"F";
  return ret;
  }

void drawFrame1(int x, int y) { // LUX
  display.setFontScale2x2(false);
  display.drawString(65 + x, 8 + y, "Luminosity");
  display.drawXbm(x , y , xbmlux_width, xbmlux_height, xbmlux_bits);
  display.setFontScale2x2(true);
  display.drawString(64 + x, 20 + y, charlux  );
}

void drawFrame2(int x, int y) { // Temperatue
  display.setFontScale2x2(false);
  display.drawString(65 + x, 8 + y, "Temperature");
  display.drawXbm(x , y , 60, 60, xbmtemp_bits);
  display.setFontScale2x2(true);
  display.drawString(64 + x, 20 + y, chartemp );
}

void drawFrame3(int x, int y) {
  display.setFontScale2x2(false);
  display.drawString(65 + x, 8 + y, "Humidity");
  display.drawXbm(x , y , xbmhum_width, xbmhum_height, xbmhum_bits);
  display.setFontScale2x2(true);
  display.drawString(64 + x, 20 + y, charhum );
}

void drawSpinner(int count, int active) {
  for (int i = 0; i < count; i++) {
    const char *xbm;
    if (active == i) {
       xbm = active_bits;
    } else {
       xbm = inactive_bits;  
    }
    display.drawXbm(64 - (12 * count / 2) + 12 * i,56, 8, 8, xbm);
  }   
}

