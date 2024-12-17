#include "my_tls_mqtt.h"
#include "esphome/core/log.h"
#include <time.h>

namespace my_tls_mqtt {

static const char *TAG = "my_tls_mqtt";

void MyTLSMQTTClient::setup() {
  printf("[INFO][%s] Setup started, waiting for WiFi...\n", TAG);

  static const char root_ca[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFBjCCAu6gAwIBAgIRAIp9PhPWLzDvI4a9KQdrNPgwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAw
WhcNMjcwMzEyMjM1OTU5WjAzMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg
RW5jcnlwdDEMMAoGA1UEAxMDUjExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB
CgKCAQEAuoe8XBsAOcvKCs3UZxD5ATylTqVhyybKUvsVAbe5KPUoHu0nsyQYOWcJ
DAjs4DqwO3cOvfPlOVRBDE6uQdaZdN5R2+97/1i9qLcT9t4x1fJyyXJqC4N0lZxG
AGQUmfOx2SLZzaiSqhwmej/+71gFewiVgdtxD4774zEJuwm+UE1fj5F2PVqdnoPy
6cRms+EGZkNIGIBloDcYmpuEMpexsr3E+BUAnSeI++JjF5ZsmydnS8TbKF5pwnnw
SVzgJFDhxLyhBax7QG0AtMJBP6dYuC/FXJuluwme8f7rsIU5/agK70XEeOtlKsLP
Xzze41xNG/cLJyuqC0J3U095ah2H2QIDAQABo4H4MIH1MA4GA1UdDwEB/wQEAwIB
hjAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwEgYDVR0TAQH/BAgwBgEB
/wIBADAdBgNVHQ4EFgQUxc9GpOr0w8B6bJXELbBeki8m47kwHwYDVR0jBBgwFoAU
ebRZ5nu25eQBc4AIiMgaWPbpm24wMgYIKwYBBQUHAQEEJjAkMCIGCCsGAQUFBzAC
hhZodHRwOi8veDEuaS5sZW5jci5vcmcvMBMGA1UdIAQMMAowCAYGZ4EMAQIBMCcG
A1UdHwQgMB4wHKAaoBiGFmh0dHA6Ly94MS5jLmxlbmNyLm9yZy8wDQYJKoZIhvcN
AQELBQADggIBAE7iiV0KAxyQOND1H/lxXPjDj7I3iHpvsCUf7b632IYGjukJhM1y
v4Hz/MrPU0jtvfZpQtSlET41yBOykh0FX+ou1Nj4ScOt9ZmWnO8m2OG0JAtIIE38
01S0qcYhyOE2G/93ZCkXufBL713qzXnQv5C/viOykNpKqUgxdKlEC+Hi9i2DcaR1
e9KUwQUZRhy5j/PEdEglKg3l9dtD4tuTm7kZtB8v32oOjzHTYw+7KdzdZiw/sBtn
UfhBPORNuay4pJxmY/WrhSMdzFO2q3Gu3MUBcdo27goYKjL9CTF8j/Zz55yctUoV
aneCWs/ajUX+HypkBTA+c8LGDLnWO2NKq0YD/pnARkAnYGPfUDoHR9gVSp/qRx+Z
WghiDLZsMwhN1zjtSC0uBWiugF3vTNzYIEFfaPG7Ws3jDrAMMYebQ95JQ+HIBD/R
PBuHRTBpqKlyDnkSHDHYPiNX3adPoPAcgdF3H2/W0rmoswMWgTlLn1Wu0mrks7/q
pdWfS6PJ1jty80r2VKsM/Dj3YIDfbjXKdaFU5C+8bhfJGqU3taKauuz0wHVGT3eo
6FlWkWYtbt4pgdamlwVeZEW+LM7qZEJEsMNPrfC03APKmZsJgpWCDWOKZvkZcvjV
uYkQ4omYCTX5ohy+knMjdOmdH9c7SpqEWBDC86fiNex+O0XOMEZSa8DA
-----END CERTIFICATE-----

)EOF";

  this->trust_anchors_ = new BearSSL::X509List(root_ca);
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

void MyTLSMQTTClient::loop() {
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

void MyTLSMQTTClient::send_discovery_message_() {
  std::string discovery_topic = "homeassistant/sensor/" + this->device_name + "/config";
  std::string payload = R"({
    "name": ")" + this->device_name + R"(",
    "state_topic": ")" + this->device_name + "/status" + R"(",
    "unique_id": ")" + this->device_name + R"(",
    "device": {
      "identifiers": [")" + this->device_name + R"("],
      "name": ")" + this->device_name + R"("
    }
  })";

  this->mqtt_client.publish(discovery_topic.c_str(), payload.c_str(), true);  // Retained message
}

void MyTLSMQTTClient::connect_to_mqtt_() {
  ESP_LOGI(TAG, "Attempting MQTT connection with authentication...");

  // Will Message setup
  this->mqtt_client.setWill((this->device_name + "/status").c_str(), "offline", true);

  if (this->mqtt_client.connect(this->broker_host.c_str(), this->broker_port, this->username.c_str(), this->password.c_str())) {
    ESP_LOGI(TAG, "MQTT connected successfully!");

    // Birth Message
    this->mqtt_client.publish((this->device_name + "/status").c_str(), "online", true);

    // Send Discovery Message
    this->send_discovery_message_();
  } else {
    ESP_LOGE(TAG, "MQTT connection failed, retrying...");
  }
}


void MyTLSMQTTClient::set_broker_host(const std::string &host) { this->broker_host = host; }
void MyTLSMQTTClient::set_broker_port(uint16_t port) { this->broker_port = port; }
void MyTLSMQTTClient::set_username(const std::string &username) { this->username_ = username; }
void MyTLSMQTTClient::set_password(const std::string &password) { this->password_ = password; }

}  // namespace my_tls_mqtt
