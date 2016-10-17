# WimIO, a 10$ temperature/humidity/Luminosity sensor.
This project is covered by Licence GPL v2.0

This station monitor temperature, humidity, and Luminosity throught wifi connection and display thoses value on a small Oled screen. This station is also connected throught MQTT protocole to create a extendable network with others stations.

The whole project is running over Arduino with some additionals module see part 4 for more info about requirement.

A special thanks to @squix78 and his fantastic weather station : https://github.com/squix78/esp8266-projects/tree/master/arduino-ide/weather-station-v2
I started to use his code and then added my own code, if you want also display weather forecast the code could easily be merge.


## Hardware requirement:
#### BOM
+ [Witty cloud ESP12F](https://fr.aliexpress.com/item/ESP8266-serial-WIFI-Witty-cloud-Development-Board-ESP-12F-module-MINI-nodemcu/32566502491.html?spm=2114.06010108.3.1.IhonA2&ws_ab_test=searchweb201556_0,searchweb201602_1_10065_10068_112_10069_110_111_418_10017_109_108_10060_10061_10062_10057_10056_10055_10054_10059_10058_10073_10072_10070_10052_10053_10050_10051,searchweb201603_4&btsid=27a84b4d-a1e6-4a92-bde2-c7923b65d879)
+ [0.93" Oled screen](https://fr.aliexpress.com/item/Free-shipping-Yellow-blue-double-color-128X64-OLED-LCD-LED-Display-Module-For-Arduino-0-96/32233367757.html?spm=2114.06010108.3.11.tMsYsi&ws_ab_test=searchweb201556_0,searchweb201602_1_10065_10068_112_10069_110_111_418_10017_109_108_10060_10061_10062_10057_10056_10055_10054_10059_10058_10073_10072_10070_10052_10053_10050_10051,searchweb201603_4&btsid=42c386d3-b9e9-4735-9921-5023d7ee0f60)
+ [DHT22 humidity/temperature sensor](https://fr.aliexpress.com/item/1pcs-DHT22-digital-temperature-and-humidity-sensor-Temperature-and-humidity-module-AM2302-replace-SHT11-SHT15-Free/1059369726.html?spm=2114.06010108.3.1.Fr8hY7&ws_ab_test=searchweb201556_0,searchweb201602_1_10065_10068_112_10069_110_111_418_10017_109_108_10060_10061_10062_10057_10056_10055_10054_10059_10058_10073_10072_10070_10052_10053_10050_10051,searchweb201603_4&btsid=23c72e47-e309-467e-a4bb-e0f7a99fc25b)
+ [TLS2561 luminosity sensor](https://fr.aliexpress.com/item/TSL2561-Luminosity-Sensor-Breakout-infrared-Light-Sensor-integrating-sensor/32600182691.html?spm=2114.06010108.3.2.AolcbO&ws_ab_test=searchweb201556_0,searchweb201602_1_10065_10068_112_10069_110_111_418_10017_109_108_10060_10061_10062_10057_10056_10055_10054_10059_10058_10073_10072_10070_10052_10053_10050_10051,searchweb201603_4&btsid=ed8dcc3c-3aac-4ee8-b985-4a517e75a9cd) 

#### Wiring
In order to get your wimIO working you just have to connect SCL pins (of screen and TLS2561) on GPIO5 of the esp8266, connect SDA on GPIO4 and data of DHT22 on GPIO2 with a 10k pull-up. 
![Wiring diagramm](https://github.com/miogui/wimio/raw/master/wimIO%20Sketch_bb.png)

## Software requirement:
Arduino + library esp8266 + adafruit TSL2561 library

#### Thingspeak
First signup for the new account in thingspeak at  [https://www.thingspeak.com](https://www.thingspeak.com)
Login to your account and  click on the channels icon.
Create a new channel by clicking on the “Create New Channel” button.
Give a name for your project and fill the field column:
+ Field1: Temperature
+ Field2: Humidty
+ Field3: Luminosity
Click the Update “Channel button” after completing the Channel Settings.
Click on the API Keys tab and copy down the API_KEY value.
Paste the API Key on Wimio.ino : String apiKey = "xxxxxxxxxxxxxxxxx";

#### Third party Library
In order to use the TSL2561 you need to add the adafruit TSL2561 library : https://github.com/adafruit/TSL2561-Arduino-Library.
This library is intend to run with standard arduino (AVR) board, to use this library with an ESP8266 board, please modify the TSL2561.cpp as follow:
suppress `#include <util/delay.h>`
change `#include <avr/pgmspace.h>` in `#include <pgmspace.h>`
add `#define _delay_ms(ms) delayMicroseconds((ms) * 1000)" just after "#include "TSL2561.h"`


#### Create your own icons
To create new icon, use gimp and export to xbm format (do not tick any trickbox)
Then open the file saved with your favorite editor and copy the content inside the icons.h file.
remove "static" descriptor ...

That's all, you can now display your icon with: `display.drawXbm(x , y , width, height, name_of_file);`
 
#### Uploading
Be sure you have settup all value needed :

`// Global Setting`

`// your network SSID (name)`

`char ssid[] = "your_ssid";`

`// your network password`

`char pass[] = "password";`

`// MQTT broker server`

`const char* mqtt_server = "broker.hivemq.com";`

`// Client name, use any name you want`

`const char* clientID = "wimio1";`

`const char* outTopic = "wimio1/outTopic";`

`const char* outTopic1 = "wimio1/Temperature";`

`const char* outTopic2 = "wimio1/Humidity";`

`const char* outTopic3 = "wimio1/Luminosity";`

`const char* inTopic = "wimio1/inTopic";`

`String apiKey = "XXXXXXXXXXXXXXXX";`

`const char* tsserver = "api.thingspeak.com";`


## Issue and future improvements
#### Issue
Actually the temperature sensor is corrupted by the esp8266 board temperature when runing with MQTT. As a workaround you need to move apart the DHT22 or reduce heating of the esp8266, by stopping polling of MQTT and put the esp8266 in low power state when the esp8266 doesn't need to run.

#### Improvements
A lot of futur developpement are already in the pipe:
+ Add an example to monitor others wimIO sensors on one wimIO (already working, just need to cleanup the code)
+ Add an example to monitor soil moisture, Co2, barometer(BMP085), etc...
+ Add a configuration file to be able to select what you want to see and monitor (humidity/temperature/luminosity/weather/finance/alert/MQTT/thingspeak/etc...)
+ Add a web-server to configure anything you want (SSID, chanel name etc...)

