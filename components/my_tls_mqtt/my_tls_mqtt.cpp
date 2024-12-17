#include "my_tls_mqtt.h"
#include <time.h>

namespace my_tls_mqtt {

static const char root_ca[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
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

// TrustAnchor wird statisch einmalig geladen
static BearSSL::X509List x509_cert(root_ca);

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
  // Starte die WLAN-Verbindung manuell
  esphome::ESP_LOGI("my_tls_mqtt", "Starting WiFi...");
  WiFi.begin();  // Explizite Initialisierung von WLAN

  // Warte auf WLAN-Verbindung
  while (!WiFi.isConnected()) {
    delay(500);
    esphome::ESP_LOGI("my_tls_mqtt", "Waiting for WiFi...");
  }
  
  esphome::ESP_LOGI("my_tls_mqtt", "WiFi connected, starting NTP sync...");

  // NTP-Synchronisation
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  int retry = 0;
  while (now < 1609459200 && retry < 30) {  // 30 Sekunden Timeout
    delay(1000);
    now = time(nullptr);
    retry++;
    esphome::ESP_LOGI("my_tls_mqtt", "Waiting for NTP time... (%d)", retry);
  }

  if (retry >= 30) {
    esphome::ESP_LOGE("my_tls_mqtt", "NTP synchronization failed, continuing without time sync!");
  } else {
    esphome::ESP_LOGI("my_tls_mqtt", "Time synchronized: %s", ctime(&now));
  }

  // TLS-Setup
  wifi_client.setTrustAnchors(&x509_cert);
  mqtt_client.setClient(wifi_client);
  //wifi_client.setInsecure();
  mqtt_client.setServer(broker_host.c_str(), broker_port);
  esphome::ESP_LOGI("my_tls_mqtt", "Free heap: %u", ESP.getFreeHeap());
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
"ESP_TLS_Client",    // Client-ID
user,    // Benutzername (optional)
pass,    // Passwort (optional)
will_topic_ptr,// Will-Topic
0, // QoS für Will-Message
true,    // Retain für Will-Message
will_payload_ptr     // Will-Payload
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
