#include "credentials.h"
#include <ESP8266MQTTMesh.h>
#include <ArduinoJson.h>

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
#if ASYNC_TCP_SSL_ENABLED
                       .setMqttSSL(mqtt_secure, mqtt_fingerprint)
#if MESH_SECURE
                       .setMeshSSL(ssl_cert, ssl_cert_len, ssl_key, ssl_key_len, ssl_fingerprint)
#endif //MESH_SECURE
#endif //ASYNC_TCP_SSL_ENABLED
                       .build();

//void callback(const char *topic, const char *msg);

void setup() {
  Serial.begin(115200);
    delay(1000); //This is only here to make it easier to catch the startup messages.  It isn't required
    //mesh.setCallback(callback);
    mesh.begin();

}

int timeSinceLastRead = 0;
void loop() {
  if (! mesh.connected())
    return;
    // Report every 2 seconds.
  if(timeSinceLastRead > 2000) {
    
    int moisVal = analogRead(0);
    // Check if any reads failed and exit early (to try again).
    if (isnan(moisVal)) {
      Serial.println("Failed to read from Soil Moisture sensor!");
      timeSinceLastRead = 0;
      return;
    }
    
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& jsonRoot = jsonBuffer.createObject();
    jsonRoot["chipId"] = ID;
    JsonObject& type = jsonRoot.createNestedObject("type");
    type["tempSensor"] = false;
    type["moisSensor"] = true;
    type["relay"] = false;
    jsonRoot["humidity"] = NULL;
    jsonRoot["temperature"] = NULL;
    jsonRoot["heatIndex"] = NULL;
    jsonRoot["moisture"] = moisVal;
    String jsonData;
    jsonRoot.printTo(jsonData);
    mesh.publish("data",jsonData.c_str());
    timeSinceLastRead = 0;
  }
  delay(100);
  timeSinceLastRead += 100;

}
