# MyTLSMQTTClient

Eine externe ESPHome-Komponente zur Verwendung von TLS-verschlüsselten MQTT-Verbindungen auf dem ESP8266.

## Konfiguration
```yaml
external_components:
  - source: github://hescher/my_tls_mqtt@main
    components: [my_tls_mqtt]

my_tls_mqtt:
  host: "broker.hivemq.com"  # MQTT-Broker-Adresse
  port: 8883                # TLS-Port
