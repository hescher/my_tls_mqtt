#include "esphome.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

namespace my_tls_mqtt {

class MyTLSMQTTClient : public esphome::Component {
 public:
  void set_host(const std::string &host, int port) {
    this->broker_host = host;
    this->broker_port = port;
  }

  void setup() override {
    wifi_client.setCACert(root_ca);  // Root-CA festlegen
    mqtt_client.setClient(wifi_client);
    mqtt_client.setServer(broker_host.c_str(), broker_port);
  }

  void loop() override {
    if (!mqtt_client.connected()) {
      reconnect();
    }
    mqtt_client.loop();
  }

 private:
  WiFiClientSecure wifi_client;
  PubSubClient mqtt_client;
  std::string broker_host;
  int broker_port;

  const char *root_ca = 
    "-----BEGIN CERTIFICATE-----\n"
    "... Root-CA hier einfügen ...\n"
    "-----END CERTIFICATE-----\n";

  void reconnect() {
    while (!mqtt_client.connected()) {
      if (mqtt_client.connect("ESP8266_TLS_Client")) {
        ESP_LOGD("TLSMQTT", "Connected to MQTT Broker over TLS");
      } else {
        delay(5000);
      }
    }
  }
};

}  // namespace my_tls_mqtt
