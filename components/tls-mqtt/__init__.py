import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import wifi  # Stellt sicher, dass WiFi geladen wird
from esphome.const import CONF_ID

DEPENDENCIES = ["wifi"]  # WLAN-Abhängigkeit sicherstellen

tls-mqtt_ns = cg.esphome_ns.namespace("tls-mqtt")
TLSMQTTClient = tls-mqtt_ns.class_("TLSMQTTClient", cg.Component)

# Eigene Konstanten definieren
CONF_HOST = "host"
CONF_PORT = "port"
CONF_USERNAME = "username"
CONF_PASSWORD = "password"
CONF_BIRTH_MESSAGE = "birth_message"
CONF_WILL_MESSAGE = "will_message"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(TLSMQTTClient),
        cv.Required(CONF_HOST): cv.string,
        cv.Required(CONF_PORT): cv.port,
        cv.Optional(CONF_USERNAME): cv.string,
        cv.Optional(CONF_PASSWORD): cv.string,
        cv.Optional(CONF_BIRTH_MESSAGE): cv.Schema(
            {
                cv.Required("topic"): cv.string,
                cv.Required("payload"): cv.string,
            }
        ),
        cv.Optional(CONF_WILL_MESSAGE): cv.Schema(
            {
                cv.Required("topic"): cv.string,
                cv.Required("payload"): cv.string,
            }
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_broker_host(config[CONF_HOST]))
    cg.add(var.set_broker_port(config[CONF_PORT]))

    if CONF_USERNAME in config:
        cg.add(var.set_username(config[CONF_USERNAME]))
    if CONF_PASSWORD in config:
        cg.add(var.set_password(config[CONF_PASSWORD]))

    if CONF_BIRTH_MESSAGE in config:
        birth_message = config[CONF_BIRTH_MESSAGE]
        cg.add(var.set_birth_message(birth_message["topic"], birth_message["payload"]))

    if CONF_WILL_MESSAGE in config:
        will_message = config[CONF_WILL_MESSAGE]
        cg.add(var.set_will_message(will_message["topic"], will_message["payload"]))
