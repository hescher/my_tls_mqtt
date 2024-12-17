#pragma once

#include "esphome.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

namespace my_tls_mqtt {

class MyTLSMQTTClient : public esphome::Component {
 public:
  void set_host(const std::string &host, int port);
  void set_username(const std::string &username);
  void set_password(const std::string &password);
  void set_discovery(bool discovery);
  void set_discovery_prefix(const std::string &prefix);
  void set_birth_message(const std::string &topic, const std::string &payload);
  void set_will_message(const std::string &topic, const std::string &payload);

  void setup() override;
  void loop() override;

 private:
  WiFiClientSecure wifi_client;
  PubSubClient mqtt_client;

  std::string broker_host;
  int broker_port;
  std::string username;
  std::string password;
  bool discovery = false;
  std::string discovery_prefix;

  std::string birth_topic;
  std::string birth_payload;
  std::string will_topic;
  std::string will_payload;

  const char *root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "... dein Root-CA-Zertifikat hier einfügen ...\n"
    "-----END CERTIFICATE-----\n";

  void reconnect();
};

}  // namespace my_tls_mqtt
