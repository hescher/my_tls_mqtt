# tls_mqttClient

Example with hivemq as tls mqtt broker.

## Konfiguration
```yaml
external_components:
  - source: github://hescher/tls_mqtt@main
    components: [tls_mqtt]

tls_mqtt:
  host: "abcdefghijklmnopqrtuvwxyz.s1.eu.hivemq.cloud"  # MQTT-Broker-Adresse
  port: 8883                                            # TLS-Port
  username: !secret hivemqtt_username                   # Hive Username
  password: !secret hivemqtt_password                   # Hive Passwort
