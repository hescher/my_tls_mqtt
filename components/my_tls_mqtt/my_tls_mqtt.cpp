#include "my_tls_mqtt.h"
#include "esphome/core/log.h"
#include <time.h>

namespace my_tls_mqtt {

static const char *TAG = "my_tls_mqtt";

void MyTLSMQTTClient::setup() {
  printf("[INFO][%s] Setup started, waiting for WiFi...\n", TAG);

  static const char root_ca[] PROGMEM = R"EOF(
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
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

void MyTLSMQTTClient::connect_to_mqtt_() {
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
}

void MyTLSMQTTClient::set_broker_host(const std::string &host) { this->broker_host = host; }
void MyTLSMQTTClient::set_broker_port(uint16_t port) { this->broker_port = port; }
void MyTLSMQTTClient::set_username(const std::string &username) { this->username_ = username; }
void MyTLSMQTTClient::set_password(const std::string &password) { this->password_ = password; }

}  // namespace my_tls_mqtt
