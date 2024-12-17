import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

# Fehlende Konstanten definieren
CONF_HOST = "host"
CONF_PORT = "port"
CONF_USERNAME = "username"
CONF_PASSWORD = "password"
CONF_DISCOVERY = "discovery"
CONF_DISCOVERY_PREFIX = "discovery_prefix"
CONF_BIRTH_MESSAGE = "birth_message"
CONF_WILL_MESSAGE = "will_message"

CODEOWNERS = ["@hescher"]
DEPENDENCIES = ["network"]

my_tls_mqtt_ns = cg.esphome_ns.namespace("my_tls_mqtt")
MyTLSMQTTClient = my_tls_mqtt_ns.class_("MyTLSMQTTClient", cg.Component)

MQTT_MESSAGE_SCHEMA = cv.Schema({
    cv.Required("topic"): cv.string,
    cv.Required("payload"): cv.string
})

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MyTLSMQTTClient),
    cv.Required(CONF_HOST): cv.string,
    cv.Required(CONF_PORT): cv.port,
    cv.Optional(CONF_USERNAME): cv.string,
    cv.Optional(CONF_PASSWORD): cv.string,
    cv.Optional(CONF_DISCOVERY, default=False): cv.boolean,
    cv.Optional(CONF_DISCOVERY_PREFIX, default="homeassistant"): cv.string,
    cv.Optional(CONF_BIRTH_MESSAGE): MQTT_MESSAGE_SCHEMA,
    cv.Optional(CONF_WILL_MESSAGE): MQTT_MESSAGE_SCHEMA,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_host(config[CONF_HOST], config[CONF_PORT]))

    if CONF_USERNAME in config:
        cg.add(var.set_username(config[CONF_USERNAME]))
    if CONF_PASSWORD in config:
        cg.add(var.set_password(config[CONF_PASSWORD]))
    
    cg.add(var.set_discovery(config[CONF_DISCOVERY]))
    cg.add(var.set_discovery_prefix(config[CONF_DISCOVERY_PREFIX]))

    if CONF_BIRTH_MESSAGE in config:
        birth = config[CONF_BIRTH_MESSAGE]
        cg.add(var.set_birth_message(birth["topic"], birth["payload"]))
    
    if CONF_WILL_MESSAGE in config:
        will = config[CONF_WILL_MESSAGE]
        cg.add(var.set_will_message(will["topic"], will["payload"]))
    
    await cg.register_component(var, config)
