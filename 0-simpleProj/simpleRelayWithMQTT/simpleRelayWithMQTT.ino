#include "credentials.h"
#include <ESP8266MQTTMesh.h>
#include <FS.h>


#define RELAYPIN 4     

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
#if ASYNC_TCP_SSL_ENABLED
                       .setMqttSSL(mqtt_secure, mqtt_fingerprint)
#if MESH_SECURE
                       .setMeshSSL(ssl_cert, ssl_cert_len, ssl_key, ssl_key_len, ssl_fingerprint)
#endif //MESH_SECURE
#endif //ASYNC_TCP_SSL_ENABLED
                       .build();

void callback(const char *topic, const char *msg);



void setup() {

    Serial.begin(115200);
    delay(1000); //This is only here to make it easier to catch the startup messages.  It isn't required
    mesh.setCallback(callback);
    mesh.begin();
    pinMode(RELAYPIN, OUTPUT);

}


void loop() {
  
}



void callback(const char *topic, const char *msg) {
Serial.println("callback içindeyim:");
Serial.println(topic);

    if (0 == strcmp(topic, (const char*) "")) {
      Serial.println("if içindeyim");
      if(String(msg) == "1") {
        Serial.println("relay pin high girdi");
        digitalWrite(RELAYPIN, HIGH);
      }else{
        Serial.println("relay pin low girdi");
        digitalWrite(RELAYPIN, LOW);
      }
    }
}
