#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "credentials.h"


#define RELAY_PIN D7
#define ON_COMMAND "on"
#define OFF_COMMAND "off"

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
Adafruit_MQTT_Publish relay_state = Adafruit_MQTT_Publish(&mqtt, "lights/D7/state");

Adafruit_MQTT_Subscribe relay_command = Adafruit_MQTT_Subscribe(&mqtt, "lights/D7");



void MQTT_connect();
void setup()
{
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  delay(1000);
  digitalWrite(RELAY_PIN, HIGH);
  Serial.begin(115200);
  delay(10);
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup MQTT subscription for relay_command feed.
  if (mqtt.subscribe(&relay_command))
    Serial.println("Subscribed");
}
uint32_t x = 0;
void loop()
{
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here
  // Here its read the subscription
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription()))
  {
    if (subscription == &relay_command)
    {
      char *message = (char *)relay_command.lastread;
      Serial.print(F("Got: "));
      Serial.println(message);
      if (!strcmp(message, ON_COMMAND))
      {
        Serial.println("relay on");
        digitalWrite(RELAY_PIN, HIGH);
      }
      else if(!strcmp(message, OFF_COMMAND))
      {
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("relay off");
      }
      else
        Serial.println("Invalid command from broker");
    
    }
    else
    {
      Serial.println("Bad read");
    }

  }
}
// Function to connect and reconnect as necessary to the MQTT server.
void MQTT_connect()
{
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected())
  {
    return;
  }
  Serial.println("Connecting to MQTT... ");
  uint8_t retries = 3;
  ret = mqtt.connect();

  while (ret != 0)
  { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println(ret);
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000); // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1)
        ;
    }
    ret = mqtt.connect();
  }
  Serial.println("MQTT Connected!");
}