#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>



// Sea Level Pressure 
#define SEALEVELPRESSURE_HPA 1013.25 

const char* ssid = "mywifi"; // Enter
const char* password = "mywifipassword"; // Enter
const char* mqtt_server = "192.168.1.xx"; // Server Address

WiFiClient espClient;
PubSubClient client(espClient);

const char* outtopic = "Sensor1/Air"; // Enter
const char* statustopic = "Sensors/Status"; // Enter

Adafruit_BME680 bme; // I2C

double humidity    = 0.0;  // Feuchtigkeit
double temperature = 0.0;  // Temperatur
double pressure    = 0.0;  // Druck
double gas         = 0.0;  // Gas/Luftqualitaet
double est_altitude= 0.0;  // Ungefaehre Hoehe

void setup_wifi() {
  Serial.print("Hello");
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    for(int i = 0; i<500; i++){
      
      delay(1);
    }
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    
    // CHANGE NAME ***********
    if (client.connect("Sensor1")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      
      // CHANGE NAME **************
      client.publish(statustopic, "Sensor1 Connected");
      
      // ... and resubscribe
     
    } else {
     
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      for(int i = 0; i<5000; i++){
        delay(1);
      }
    }
  }
}
void setup()
{
  Serial.begin(9600); // seriellen Monitor starten
  
  if (!bme.begin(0x76)) {
    Serial.println("No BME680 Sensor found!");
    while (1);
  }
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  delay(1000); 
  
  
  // Initialisierung von  Oversampling und Filter
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}


void loop()
{
  measurement();      // Messwerte erfassen
}

void measurement(void)
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Erst den bme680 auslesen
  if (! bme.performReading()) {
    Serial.println("Fehler beim Messen ");
    return;
  }
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  // CHANGE NAME ******************1
  root["device"] = "Sensor1ESP8266";
  
  
  // Werte ermitteln:
  temperature   = bme.temperature;
  pressure      = bme.pressure / 100.0;
  humidity      = bme.humidity;
  gas           = bme.gas_resistance / 1000.0;
  est_altitude  = bme.readAltitude(SEALEVELPRESSURE_HPA);

  // add values to json buffer
  root["temperature"] = temperature;
  root["humidity"] = humidity;
  root["pressure"] = pressure;
  root["gas"] = gas;

  Serial.print("Temperatur = ");
  Serial.print(temperature);
  Serial.println(" *C");
  
  Serial.print("Luftdruck = ");
  Serial.print(pressure);
  Serial.println(" hPa");

  Serial.print("Feuchtigkeit = ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(gas);
  Serial.println(" KOhms");

  Serial.print("Ungefaehre Hoehe = ");
  Serial.print(est_altitude);
  Serial.println(" m");

  char JSONmessageBuffer[100];
  root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  if (client.publish(outtopic, JSONmessageBuffer) == true) {
    Serial.println("Success sending message");
  } else {
    Serial.println("Error sending message");
  }
 

  Serial.println();
  delay(30000);
}
