#include "my_tls_mqtt.h"
#include "esphome/core/log.h"
#include <time.h>  // Für die NTP-Synchronisation

namespace my_tls_mqtt {

static const char *TAG = "my_tls_mqtt";

const char root_ca[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
...
-----END CERTIFICATE-----
)EOF";

void MyTLSMQTTClient::setup() {
  ESP_LOGI(TAG, "Setup started, waiting for WiFi...");

  // Initialisiere den TLS-Client und setze das Zertifikat
  this->trust_anchors_ = new BearSSL::X509List(root_ca);
  this->wifi_client.setTrustAnchors(this->trust_anchors_);
  
  this->mqtt_client.setClient(this->wifi_client);
  this->mqtt_client.setServer(this->broker_host.c_str(), this->broker_port);

  this->initialized_ = false;
}

void MyTLSMQTTClient::loop() {
  if (!WiFi.isConnected()) {
    ESP_LOGW(TAG, "WiFi not connected, waiting...");
    return;
  }

  if (!this->initialized_) {
    ESP_LOGI(TAG, "WiFi connected! Starting NTP sync...");

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    time_t now = time(nullptr);
    int retry = 0;

    while (now < 1609459200 && retry < 30) {  // 30 Sekunden Timeout
      delay(1000);
      now = time(nullptr);
      ESP_LOGI(TAG, "Waiting for NTP time... (%d)", retry++);
    }

    if (retry >= 30) {
      ESP_LOGE(TAG, "NTP synchronization failed, continuing without time sync!");
    } else {
      ESP_LOGI(TAG, "Time synchronized: %s", ctime(&now));
    }

    this->initialized_ = true;
    this->connect_to_mqtt_();
  }

  if (!this->mqtt_client.connected()) {
    ESP_LOGW(TAG, "MQTT disconnected, reconnecting...");
    this->connect_to_mqtt_();
  } else {
    this->mqtt_client.loop();
  }
}

void MyTLSMQTTClient::connect_to_mqtt_() {
  if (this->username_.empty() || this->password_.empty()) {
    ESP_LOGI(TAG, "Attempting MQTT connection without authentication...");
    if (!this->mqtt_client.connect("esphome_client")) {
      ESP_LOGE(TAG, "MQTT connection failed, retrying...");
      return;
    }
  } else {
    ESP_LOGI(TAG, "Attempting MQTT connection with authentication...");
    if (!this->mqtt_client.connect("esphome_client", this->username_.c_str(), this->password_.c_str())) {
      ESP_LOGE(TAG, "MQTT connection failed, retrying...");
      return;
    }
  }

  ESP_LOGI(TAG, "MQTT connected successfully!");
}

void MyTLSMQTTClient::set_broker_host(const std::string &host) { this->broker_host = host; }
void MyTLSMQTTClient::set_broker_port(uint16_t port) { this->broker_port = port; }
void MyTLSMQTTClient::set_username(const std::string &username) { this->username_ = username; }
void MyTLSMQTTClient::set_password(const std::string &password) { this->password_ = password; }

}  // namespace my_tls_mqtt
