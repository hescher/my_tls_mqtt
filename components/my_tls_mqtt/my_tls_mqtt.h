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
  void setup() override;
  void loop() override;

 private:
  WiFiClientSecure wifi_client;
  PubSubClient mqtt_client;

  std::string broker_host;
  int broker_port;
  std::string username;
  std::string password;

  const char *root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "... dein Root-CA-Zertifikat hier einfügen ...\n"
    "-----END CERTIFICATE-----\n";

  void reconnect();
};

}  // namespace my_tls_mqtt
