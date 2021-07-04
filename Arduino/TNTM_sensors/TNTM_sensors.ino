/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/


#include <WiFi.h>

#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "linksys-Srebrin";
const char* password = "9D81BB8721";
const char* mqtt_server = "3.67.237.255";//"192.168.1.10";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (150)
char msg[MSG_BUFFER_SIZE];
int value = 0;


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

 // WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
//End Setup WiFi 
// Init Temperature DS18B20 OneWire


#include <OneWire.h>
#include <DallasTemperature.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = 32;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

void setup_sensor(){
  sensors.begin();
}
//End Temperature DS18B20 OneWire
//Gas
#include "MQ135.h"
/* https://github.com/espressif/arduino-esp32/issues/102
  This happens because ADC2 pins can not be used when WiFi is used.
   On the other hand, ADC1 pins can be used even when WiFi is enabled.
 */
///https://www.electroduino.com/air-pollution-monitoring-system-using-arduino-and-mq135-air-quality-sensor/
int Gas_analog = 33;    // used for ESP32
int Gas_digital = 35;   // used for ESP32
MQ135 gasSensor = MQ135(Gas_analog);
void setup_gas(){ 
 // pinMode(gasSensor,INPUT);
 // adcAttachPin(gasSensor);
 // analogReadResolution(11);
 // analogSetAttenuation(ADC_6db);
 
  pinMode(Gas_digital, INPUT);
}
//End Gas

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
/*
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
    delay(5000);
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
*/
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
//  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 
  setup_sensor();

  setup_gas();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 1000) {
    float air_quality = gasSensor.getPPM();
    float air_RZero = gasSensor.getRZero();
    float org_gassensorAnalog =analogRead(Gas_analog);
    float gassensorAnalog = org_gassensorAnalog * 0.004882814;
    int gassensorDigital = digitalRead(Gas_digital);
    
    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    Serial.println(temperatureC);
    lastMsg = now;
    ++value;
   // snprintf (msg, MSG_BUFFER_SIZE, "{\"s1\":{\"t\":\"%ld\",\"id\":\"#%ld\",\"tC\":%f}}", now,value,temperatureC);
   // snprintf (msg, MSG_BUFFER_SIZE, "{\"s1\":\"%ld,#%ld,%f\"}", now,value,temperatureC);
    snprintf (msg, MSG_BUFFER_SIZE, "\"%ld\",\"#%ld\",%f,%f,%f,%f,%f,%d", now,value,temperatureC,org_gassensorAnalog,air_quality,air_RZero,gassensorAnalog,gassensorDigital);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("srebrinb/sensor/outTemC", msg,false);
  }
  
}
