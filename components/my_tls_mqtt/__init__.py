import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

# Fehlende Konstanten definieren
CONF_HOST = "host"
CONF_PORT = "port"
CONF_USERNAME = "username"
CONF_PASSWORD = "password"

CODEOWNERS = ["@hescher"]
DEPENDENCIES = ["network"]

my_tls_mqtt_ns = cg.esphome_ns.namespace("my_tls_mqtt")
MyTLSMQTTClient = my_tls_mqtt_ns.class_("MyTLSMQTTClient", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MyTLSMQTTClient),
    cv.Required(CONF_HOST): cv.string,
    cv.Required(CONF_PORT): cv.port,
    cv.Optional(CONF_USERNAME): cv.string,   # Optionaler Benutzername
    cv.Optional(CONF_PASSWORD): cv.string,   # Optionales Passwort
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_host(config[CONF_HOST], config[CONF_PORT]))
    
    # Optional: Benutzername und Passwort setzen
    if CONF_USERNAME in config:
        cg.add(var.set_username(config[CONF_USERNAME]))
    if CONF_PASSWORD in config:
        cg.add(var.set_password(config[CONF_PASSWORD]))
    
    await cg.register_component(var, config)
