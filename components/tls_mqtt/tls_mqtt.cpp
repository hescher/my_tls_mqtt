#include "tls_mqtt.h"
#include "esphome/core/log.h"
#include <time.h>

namespace tls_mqtt {

static const char *TAG = "tls_mqtt";

void TLSMQTTClient::setup() {
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
    printf(TAG, "Published test message to topic: %s, payload: %s", topic.c_str(), payload.c_str());
  } else {
    printf(TAG, "MQTT client not connected. Cannot publish message.");
  }
}

void TLSMQTTClient::connect_to_mqtt_() {
  printf("[INFO][%s] Attempting MQTT connection...\n", TAG);

  bool connected = false;

  if (!this->will_topic_.empty() && !this->will_payload_.empty()) {
    // Mit Last-Will-and-Testament (LWT)
    if (this->username_.empty() || this->password_.empty()) {
      // Ohne Authentifizierung
      connected = this->mqtt_client.connect("esphome_client", this->will_topic_.c_str(), 0, true, this->will_payload_.c_str());
    } else {
      // Mit Authentifizierung
      connected = this->mqtt_client.connect("esphome_client", this->username_.c_str(), this->password_.c_str(),
                                            this->will_topic_.c_str(), 0, true, this->will_payload_.c_str());
    }
  } else {
    // Ohne Last-Will-and-Testament
    if (this->username_.empty() || this->password_.empty()) {
      // Ohne Authentifizierung
      connected = this->mqtt_client.connect("esphome_client");
    } else {
      // Mit Authentifizierung
      connected = this->mqtt_client.connect("esphome_client", this->username_.c_str(), this->password_.c_str());
    }
  }

  if (connected) {
    printf("[INFO][%s] MQTT connected successfully!\n", TAG);
    // Sende Birth Message, falls konfiguriert
    if (!this->birth_topic_.empty() && !this->birth_payload_.empty()) {
      this->publish_message(this->birth_topic_, this->birth_payload_);
      printf("[INFO][%s] Sent Birth Message to topic: %s\n", TAG, this->birth_topic_.c_str());
    }
  } else {
    printf("[ERROR][%s] MQTT connection failed, retrying...\n", TAG);
  }
}

void TLSMQTTClient::set_broker_host(const std::string &host) { this->broker_host = host; }
void TLSMQTTClient::set_broker_port(uint16_t port) { this->broker_port = port; }
void TLSMQTTClient::set_username(const std::string &username) { this->username_ = username; }
void TLSMQTTClient::set_password(const std::string &password) { this->password_ = password; }
void TLSMQTTClient::set_birth_message(const std::string &topic, const std::string &payload) {
  this->birth_topic_ = topic;
  this->birth_payload_ = payload;
}

void TLSMQTTClient::set_will_message(const std::string &topic, const std::string &payload) {
  this->will_topic_ = topic;
  this->will_payload_ = payload;
}

void TLSMQTTClient::log_message(const std::string &level, const std::string &message) {
  if (!this->mqtt_client.connected()) {
    printf(TAG, "MQTT client not connected. Cannot log message.");
    return;
  }

  std::string topic = "homeassistant/logs/" + level;
  this->mqtt_client.publish(topic.c_str(), message.c_str());
}

}  // namespace tls_mqtt
