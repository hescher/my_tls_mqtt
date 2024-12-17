# TLSMQTTClient


## Konfiguration
```yaml
external_components:
  - source: github://hescher/tls_mqtt@main
    components: [tls_mqtt]

my_tls_mqtt:
  host: "broker.hivemq.com"  # MQTT-Broker-Adresse
  port: 8883                # TLS-Port
