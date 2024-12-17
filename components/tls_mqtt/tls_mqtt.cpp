#include "tls_mqtt.h"
#include "esphome/core/log.h"
#include <time.h>

namespace tls_mqtt {

static const char *TAG = "tls_mqtt";

void TLSMQTTClient::setup(const char *cert) {
  printf("[INFO][%s] Setup started, waiting for WiFi...\n", TAG);

  this->trust_anchors_ = new BearSSL::X509List(cert);
  if (!this->trust_anchors_) {
    printf("[ERROR][%s] Failed to load Root CA certificate!\n", TAG);
    return;
  }

  printf("[INFO][%s] Root CA certificate loaded successfully.\n", TAG);

  this->wifi_client.setTrustAnchors(this->trust_anchors_);
  
  // MQTT-Client initialisieren
  this->mqtt_client.setClient(this->wifi_client);
  this->mqtt_client.setServer(this->broker_host.c_str(), this->broker_port);

  this->initialized_ = false;
}

void TLSMQTTClient::loop() {
  if (!WiFi.isConnected()) {
    printf("[WARN][%s] WiFi not connected, waiting...\n", TAG);
    return;
  }

  if (!this->initialized_) {
    printf("[INFO][%s] WiFi connected! Starting NTP sync...\n", TAG);

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    time_t now = time(nullptr);
    int retry = 0;

    while (now < 1609459200 && retry < 30) {
      delay(1000);
      now = time(nullptr);
      printf("[INFO][%s] Waiting for NTP time... (%d)\n", TAG, retry++);
    }

    if (retry >= 30) {
      printf("[ERROR][%s] NTP synchronization failed, continuing without time sync!\n", TAG);
    } else {
      printf("[INFO][%s] Time synchronized: %s", TAG, ctime(&now));
    }

    this->initialized_ = true;
    this->connect_to_mqtt_();
  }

  if (!this->mqtt_client.connected()) {
    printf("[WARN][%s] MQTT disconnected, reconnecting...\n", TAG);
    this->connect_to_mqtt_();
  } else {
    this->mqtt_client.loop();
  }
}

void TLSMQTTClient::publish_message(const std::string &topic, const std::string &payload) {
  if (this->mqtt_client.connected()) {
    this->mqtt_client.publish(topic.c_str(), payload.c_str());
    esphome::ESP_LOGI(TAG, "Published test message to topic: %s, payload: %s", topic.c_str(), payload.c_str());
  } else {
    esphome::ESP_LOGW(TAG, "MQTT client not connected. Cannot publish message.");
  }
}

void TLSMQTTClient::connect_to_mqtt_() {
  if (this->username_.empty() || this->password_.empty()) {
    printf("[INFO][%s] Attempting MQTT connection without authentication...\n", TAG);
    if (!this->mqtt_client.connect("esphome_client")) {
      printf("[ERROR][%s] MQTT connection failed, retrying...\n", TAG);
      return;
    }
  } else {
    printf("[INFO][%s] Attempting MQTT connection with authentication...\n", TAG);
    if (!this->mqtt_client.connect("esphome_client", this->username_.c_str(), this->password_.c_str())) {
      printf("[ERROR][%s] MQTT connection failed, retrying...\n", TAG);
      return;
    }
  }
  printf("[INFO][%s] MQTT connected successfully!\n", TAG);
  this->publish_message("test/topic", "Hello, MQTT!");
  
}

void TLSMQTTClient::set_broker_host(const std::string &host) { this->broker_host = host; }
void TLSMQTTClient::set_broker_port(uint16_t port) { this->broker_port = port; }
void TLSMQTTClient::set_username(const std::string &username) { this->username_ = username; }
void TLSMQTTClient::set_password(const std::string &password) { this->password_ = password; }

}  // namespace tls_mqtt
