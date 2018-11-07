// IFTTT/Adafruit IO/NeoPixel strio/7-segment display weather shadowbox by Becky Stern
// This program contains bits and pieces of various library sample codes:
/*************************************************** 
  Adafruit LED 7-Segment backpacks 
  ----> http://www.adafruit.com/products/881
  ----> http://www.adafruit.com/products/880
  ----> http://www.adafruit.com/products/879
  ----> http://www.adafruit.com/products/878

  These displays use I2C to communicate, 2 pins are required to 
  interface. There are multiple selectable I2C addresses. For backpacks
  with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
  with 3 Address Select pins: 0x70 thru 0x77

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  I2C LED Backpack Example written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
  
// Adafruit IO Multiple Feed Example written by Todd Treece for Adafruit Industries
// Smart Toilet Light with ESP8266 written by Tony DiCola for Adafruit Industries
// Licenses: MIT (https://opensource.org/licenses/MIT)
 ****************************************************/

#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>

#include <WiFi.h>
#include <AdafruitIO.h>
#include <Adafruit_MQTT.h>

// Adafruit IO Subscription Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

// before running this code, create feeds on Adafruit IO that match these names:
AdafruitIO_Feed *hightemp = io.feed("hightemp"); // set up the 'hightemp' feed
AdafruitIO_Feed *precipitation = io.feed("precipitation"); // set up the 'precipitation' feed

void setSevSeg(int n, int s);
void setSingleSevSeg(int device, char n);
void SevSegPowerOff();
void SevSegPowerOn();
void initSevSeg();


const char SEVSEG_nPINS[] = { 21, 17, 16, 19, 18, 5, 4};
const char SEVSEG_sPINS[] = { 15, 33, 27, 12};
const bool SEVSEG_MAP[10][7] = {
  //     GFEDCBA  Segments      7-segment map:
  { 0,1,1,1,1,1,1 }, // 0   "0"
  { 0,0,0,0,1,1,0 }, // 1   "1"
  { 1,0,1,1,0,1,1 }, // 2   "2"
  { 1,0,0,1,1,1,1 }, // 3   "3"
  { 1,1,0,0,1,1,0 }, // 4   "4"
  { 1,1,0,1,1,0,1 }, // 5   "5"
  { 1,1,1,1,1,0,1 }, // 6   "6"
  { 0,0,0,0,1,1,1 }, // 7   "7"
  { 1,1,1,1,1,1,1 }, // 8   "8"
  { 1,1,0,1,1,1,1 }  // 9   "9"
};

void setup() {
  // start the serial connection
  Serial.begin(115200);

  //pin 12 on sevseg is 12 on huzzah, 5 on uno
  //pin 11 on sevseg is A4 on huzzah, 12 on uno
  //pin 10 on sevseg is A5 on huzzah, 8 on uno
  //pin 9 on sevseg is SCK on huzzah, 13 on uno
  //pin 8 on sevseg is MO on huzzah, 9 on uno
  //pin 7 on sevseg is MI on huzzah, 10 on uno
  //pin 6 on sevseg is MO on huzzah, 7 on uno
  //pin 5 on sevseg is 27 on huzzah, 4 on uno
  //pin 4 on sevseg is 33 on huzzah, 3 on uno
  //pin 3 on sevseg is  on huzzah, 11 on uno
  //pin 2 on sevseg is  on huzzah, 6 on uno
  //pin 1 on sevseg is 15 on huzzah, 2 on uno

  //sevseg.Begin(1,15,33,27,12,6,7,8,9,10,11,12,13);
  //sevseg.Brightness(90);
  //matrix.print(10, DEC);  // send the temperature value to the display
  //matrix.writeDisplay();          // light up display
  initSevSeg();
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  // set up a message handler for the count feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  hightemp->onMessage(handleTemp);
  precipitation->onMessage(handleCondition);
  setSevSeg(9999, 10);
  // wait for a connection
  {     // animation while waiting for connection; removes need for serial connectivity
    SevSegPowerOff();
    for (int i=0; i<4; i++) {
      digitalWrite(SEVSEG_sPINS[i], 0);
      delay(500);
    }
    int i = 0;
    while(io.status() < AIO_CONNECTED) {
      Serial.print(".");
      digitalWrite(SEVSEG_nPINS[i%7], ((i/7)+1) %2);
      delay(500);
      i++;
    }
    for (int i=0; i<10; i++) {
      SevSegPowerOn();
      delay(250);
      SevSegPowerOff();
      delay(250);
    }
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

}

void loop() {
  
  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();
}

// this function is called whenever a feed message
// is received from Adafruit IO. it was attached to
// the feed in the setup() function above.
void handleTemp(AdafruitIO_Data *data) {

  Serial.print("received <- ");
  Serial.println(data->value());  // print the temperature data to the serial monitor
  
  int todaysHigh = data->toInt(); // store the incoming temperature data as an integer
  setSevSeg(todaysHigh, 30); // set the display for 30 second
}  
  
void handleCondition(AdafruitIO_Data *data) {
  
  String forecast = data->toString(); // store the incoming weather data in a string
  
  //the following strings store the varous IFTTT weather report words I've discovered so far
  String rain = String("Rain");
  String lightrain = String("Light Rain");
  String rainshower = String ("Rain Shower");
  String snow = String("Snow");
  String cloudy = String("Cloudy");
  String mostlycloudy = String("Mostly Cloudy");
  String partlycloudy = String("Partly Cloudy");
  String clearsky = String("Clear");
  String fair = String("Fair");
  String sunny = String("Sunny");
  String rainandsnow = String("Rain and Snow");
  String snowshower = String("Snow Shower");
  
  // if there's rain in the forecast, Blue
  if (forecast.equalsIgnoreCase(rain) || forecast.equalsIgnoreCase(lightrain) || forecast.equalsIgnoreCase(rainshower)){
    Serial.println("precipitation in the forecast today");
    digitalWrite(LED_RED, 0);
    digitalWrite(LED_GREEN, 0);
    digitalWrite(LED_BLUE, 1);
  }
  
  // if there's snow in the forecast, Teal
  if (forecast.equalsIgnoreCase(snow) || forecast.equalsIgnoreCase(rainandsnow) || forecast.equalsIgnoreCase(snowshower)){
    Serial.println("precipitation in the forecast today");
    digitalWrite(LED_RED, 0);
    digitalWrite(LED_GREEN, 1);
    digitalWrite(LED_BLUE, 1);
  }
  
  // if there's sun in the forecast, Yellow
  if (forecast.equalsIgnoreCase(clearsky) || forecast.equalsIgnoreCase(fair) || forecast.equalsIgnoreCase(sunny)){
    Serial.println("some kind of sun in the forecast today");
    digitalWrite(LED_RED, 1);
    digitalWrite(LED_GREEN, 1);
    digitalWrite(LED_BLUE, 0);
  }
  // Cloudy, White
  if (forecast.equalsIgnoreCase(cloudy) || forecast.equalsIgnoreCase(mostlycloudy) || forecast.equalsIgnoreCase(partlycloudy)){
    Serial.println("cloudy sky in the forecast today");
    digitalWrite(LED_RED, 1);
    digitalWrite(LED_GREEN, 1);
    digitalWrite(LED_BLUE, 1);
   }
}

void setSevSeg(int n, int s) {
  const int onMS = 2;
  if ( n < 10000 && n > 0 ){
    char num[] = { n % 10,
                    n /10 %10,
                    n /100 %10,
                    n /1000 };
    for (int i=0; i<s*(1000/onMS); i++) {
      setSingleSevSeg(i%4, num[i%4]);
      delay(onMS);
    }
  }
}

void setSingleSevSeg(int device, char n) {
  SevSegPowerOff();
  for (int i=0; i<7; i++) {
    digitalWrite(SEVSEG_nPINS[i], SEVSEG_MAP[n][i]);
  }
  digitalWrite(SEVSEG_sPINS[device], 0);
}

void SevSegPowerOff() {
  for (int i=0; i<4; i++) {
    digitalWrite(SEVSEG_sPINS[i], 1);
  }
  for (int i=0; i<7; i++) {
    digitalWrite(SEVSEG_nPINS[i], 0);
  }
}

void SevSegPowerOn() {
  for (int i=0; i<4; i++) {
    digitalWrite(SEVSEG_sPINS[i], 0);
  }
  for (int i=0; i<7; i++) {
    digitalWrite(SEVSEG_nPINS[i], 1);
  }
}

void initSevSeg() {
  for (int i=0; i<4; i++) {
    pinMode(SEVSEG_sPINS[i], OUTPUT);
  }
  for (int i=0; i<7; i++) {
    pinMode(SEVSEG_nPINS[i], OUTPUT);
  }
  SevSegPowerOff();
}
