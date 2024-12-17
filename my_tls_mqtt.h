#include "esphome.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

class MyTLSMQTTClient : public Component {
 public:
  void setup() override {
    WiFiClientSecure wifiClient;
    wifiClient.setCACert(root_ca);  // Zertifikat hinzufügen

    mqttClient.setClient(wifiClient);
    mqttClient.setServer("broker.hivemq.com", 8883);  // HiveMQ Broker + Port
  }

  void loop() override {
    if (!mqttClient.connected()) {
      reconnect();
    }
    mqttClient.loop();
  }

 private:
  PubSubClient mqttClient;
  const char *root_ca = \
    "-----BEGIN CERTIFICATE-----\n"
    "... hier dein Root-CA-Zertifikat einfügen ...\n"
    "-----END CERTIFICATE-----\n";

  void reconnect() {
    while (!mqttClient.connected()) {
      if (mqttClient.connect("ESP8266_Client")) {
        ESP_LOGD("TLSMQTT", "Connected to HiveMQ over TLS");
      } else {
        delay(5000);
      }
    }
  }
};
