# TLSMQTTClient


## Konfiguration
```yaml
external_components:
  - source: github://hescher/tls-mqtt@main
    components: [tls-mqtt]

tls-mqtt:
  host: "broker.hivemq.com"  # MQTT-Broker-Adresse
  port: 8883                # TLS-Port
