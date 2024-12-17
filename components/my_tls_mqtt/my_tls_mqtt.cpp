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
  // TLS-Zertifikat setzen
  static BearSSL::X509List cert(root_ca);
  wifi_client.setTrustAnchors(&cert);

  // MQTT-Client konfigurieren
  mqtt_client.setClient(wifi_client);
  mqtt_client.setServer(broker_host.c_str(), broker_port);
}

void MyTLSMQTTClient::loop() {
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();
}

void MyTLSMQTTClient::reconnect() {
  while (!mqtt_client.connected()) {
    esphome::ESP_LOGI("my_tls_mqtt", "Attempting MQTT connection to %s...", broker_host.c_str());

    const char* user = username.empty() ? nullptr : username.c_str();
    const char* pass = password.empty() ? nullptr : password.c_str();
    const char* will_topic_ptr = will_topic.empty() ? nullptr : will_topic.c_str();
    const char* will_payload_ptr = will_payload.empty() ? nullptr : will_payload.c_str();

    bool connected = mqtt_client.connect(
      "ESP_TLS_Client",          // Client-ID
      user,                      // Benutzername (optional)
      pass,                      // Passwort (optional)
      will_topic_ptr,            // Will-Topic
      0,                         // QoS für Will-Message
      true,                      // Retain für Will-Message
      will_payload_ptr           // Will-Payload
    );

    if (connected) {
      esphome::ESP_LOGI("my_tls_mqtt", "Connected to MQTT Broker: %s", broker_host.c_str());
      
      // Birth-Message senden
      if (!birth_topic.empty() && !birth_payload.empty()) {
        mqtt_client.publish(birth_topic.c_str(), birth_payload.c_str(), true);
        esphome::ESP_LOGI("my_tls_mqtt", "Sent birth message to %s", birth_topic.c_str());
      }
    } else {
      esphome::ESP_LOGE("my_tls_mqtt", "MQTT connection failed, retrying in 5 seconds...");
      delay(5000);
    }
  }
}


}  // namespace my_tls_mqtt
