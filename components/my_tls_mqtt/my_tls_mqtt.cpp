#include "my_tls_mqtt.h"

namespace my_tls_mqtt {

void MyTLSMQTTClient::set_host(const std::string &host, int port) {
  this->broker_host = host;
  this->broker_port = port;
}

void MyTLSMQTTClient::set_username(const std::string &username) {
  this->username = username;
}

void MyTLSMQTTClient::set_password(const std::string &password) {
  this->password = password;
}

void MyTLSMQTTClient::set_discovery(bool discovery) {
  this->discovery = discovery;
}

void MyTLSMQTTClient::set_discovery_prefix(const std::string &prefix) {
  this->discovery_prefix = prefix;
}

void MyTLSMQTTClient::set_birth_message(const std::string &topic, const std::string &payload) {
  this->birth_topic = topic;
  this->birth_payload = payload;
}

void MyTLSMQTTClient::set_will_message(const std::string &topic, const std::string &payload) {
  this->will_topic = topic;
  this->will_payload = payload;
}

void MyTLSMQTTClient::setup() {
  static BearSSL::X509List cert(root_ca);
  wifi_client.setTrustAnchors(&cert);

  mqtt_client.setClient(wifi_client);
  mqtt_client.setServer(broker_host.c_str(), broker_port);

  // Birth- und Will-Message konfigurieren
  if (!will_topic.empty() && !will_payload.empty()) {
    mqtt_client.setWill(will_topic.c_str(), will_payload.c_str(), true, 0);
  }
}

void MyTLSMQTTClient::loop() {
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();
}

void MyTLSMQTTClient::reconnect() {
  while (!mqtt_client.connected()) {
    if (mqtt_client.connect("ESP_TLS_Client", username.c_str(), password.c_str(),
                            will_topic.c_str(), 0, true, will_payload.c_str())) {
      esphome::ESP_LOGD("my_tls_mqtt", "Connected to MQTT Broker: %s", broker_host.c_str());

      // Birth-Message senden
      if (!birth_topic.empty() && !birth_payload.empty()) {
        mqtt_client.publish(birth_topic.c_str(), birth_payload.c_str(), true);
      }
    } else {
      delay(5000);
    }
  }
}

}  // namespace my_tls_mqtt
