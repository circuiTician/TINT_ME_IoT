#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//Relays for switching appliances
#define Relay1            2
#define Relay2            14

#define WLAN_SSID       "**************"
#define WLAN_PASS       "**************"

// Adafruit.io Setup 

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   
#define AIO_USERNAME    "**************"
#define AIO_KEY         "**************"


WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);


// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/hum");
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe Data1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Data1");
Adafruit_MQTT_Subscribe Data2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Data2");
unsigned long pm;

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  digitalWrite(Relay1, 1);
  digitalWrite(Relay2, 1);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
 
  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&Data1);
  mqtt.subscribe(&Data2);
}


void loop() 
{
    MQTT_connect();
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(20000))) 
      {
        if (subscription == &Data1) 
          {
            Serial.print(F("Got: "));
            Serial.println((char *)Data1.lastread);
            int Data1_State = atoi((char *)Data1.lastread);
            digitalWrite(Relay1, !Data1_State);
          }
        
        if (subscription == &Data2) 
          {
            Serial.print(F("Got: "));
            Serial.println((char *)Data2.lastread);
            int Data2_State = atoi((char *)Data2.lastread);
            digitalWrite(Relay2, !Data2_State);
          }
      }

    if (millis()-pm >= 2000)
      {
        float h = random(0,100);
        float t = random(0,100);

        if (! Temperature.publish(t)) {  Serial.println(F("Failed to publish temperature feed")); } 
        else                          { Serial.println(F("Succesfully published to temperature feed")); }

        if (! Humidity.publish(h)) {  Serial.println("Failed to publish temperature feed"); } 
        else                       { Serial.println("Succesfully published to humidity feed"); }
        pm=millis();
      }


  }


void MQTT_connect()   
  {
    int8_t ret;

    // Stop if already connected.
    if (mqtt.connected()) { return; }

    Serial.print("Connecting to MQTT... ");

    uint8_t retries = 3;
   
    while ((ret = mqtt.connect()) != 0) 
      { // connect will return 0 for connected
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying MQTT connection in 5 seconds...");
        mqtt.disconnect();
        delay(5000);  // wait 5 seconds
        retries--;

        if (retries == 0)  { while (1); {} }
      }
    Serial.println("MQTT Connected!");
   
  }

