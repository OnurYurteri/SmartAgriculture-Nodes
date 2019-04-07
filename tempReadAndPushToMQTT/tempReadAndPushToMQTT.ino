#include "credentials.h"
#include <ESP8266MQTTMesh.h>
#include <FS.h>
#include <ArduinoJson.h>

//TEMP SENSOR
#include "DHT.h"
#define DHTPIN 4     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

#ifndef LED_PIN
  #define LED_PIN LED_BUILTIN
#endif

//MESH
#define      FIRMWARE_ID        0x1337
#define      FIRMWARE_VER       "0.1"
wifi_conn    networks[]       = NETWORK_LIST;
const char*  mesh_password    = MESH_PASSWORD;
const char*  mqtt_server      = MQTT_SERVER;
const int    mqtt_port        = MQTT_PORT;
#if ASYNC_TCP_SSL_ENABLED
const uint8_t *mqtt_fingerprint = MQTT_FINGERPRINT;
bool mqtt_secure = MQTT_SECURE;
  #if MESH_SECURE
  #include "ssl_cert.h"
  #endif
#endif

#ifdef ESP32
String ID  = String((unsigned long)ESP.getEfuseMac());
#else
String ID  = String(ESP.getChipId());
#endif


// Note: All of the '.set' options below are optional.  The default values can be
// found in ESP8266MQTTMeshBuilder.h
ESP8266MQTTMesh mesh = ESP8266MQTTMesh::Builder(networks, mqtt_server, mqtt_port)
                       .setVersion(FIRMWARE_VER, FIRMWARE_ID)
                       .setMeshPassword(mesh_password)
                       //.setMqttAuth("mqttUser", "extremePass")
#if ASYNC_TCP_SSL_ENABLED
                       .setMqttSSL(mqtt_secure, mqtt_fingerprint)
#if MESH_SECURE
                       .setMeshSSL(ssl_cert, ssl_cert_len, ssl_key, ssl_key_len, ssl_fingerprint)
#endif //MESH_SECURE
#endif //ASYNC_TCP_SSL_ENABLED
                       .build();

void callback(const char *topic, const char *msg);

//DHT
DHT dht(DHTPIN, DHTTYPE);


void setup() {

    Serial.begin(115200);
    delay(1000); //This is only here to make it easier to catch the startup messages.  It isn't required
    mesh.setCallback(callback);
    mesh.begin();
    pinMode(LED_PIN, OUTPUT);

}

int timeSinceLastRead = 0;
void loop() {


  if (! mesh.connected())
    return;
    // Report every 2 seconds.
  if(timeSinceLastRead > 2000) {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      timeSinceLastRead = 0;
      return;
    }
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& jsonRoot = jsonBuffer.createObject();
    jsonRoot["chipId"] = ID;
    JsonObject& type = jsonRoot.createNestedObject("type");
    type["tempSensor"] = true;
    type["moisSensor"] = false;
    type["relay"] = false;
    jsonRoot["humidity"] = h;
    jsonRoot["temperature"] = t;
    jsonRoot["heatIndex"] = hic;
    jsonRoot["moisture"] = NULL;
    String jsonData;
    jsonRoot.printTo(jsonData);
    /*
    String msg = "ID: " + ID + 
                 " %\t Nem: " + h + 
                 " %\t Temperature: " + t + " *C " +
                 " %\t Heat index: " + hic + " *C " ;
    */
    mesh.publish("data",jsonData.c_str(), MSG_TYPE_QOS_2);
    timeSinceLastRead = 0;
  }
  delay(100);
  timeSinceLastRead += 100;
}



void callback(const char *topic, const char *msg) {


    if (0 == strcmp(topic, (const char*) ID.c_str())) {
      if(String(msg) == "0") {
        digitalWrite(LED_PIN, HIGH);
      }else{
        digitalWrite(LED_PIN, LOW);
      }
    }
}
