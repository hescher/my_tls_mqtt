import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_HOST, CONF_PORT

CODEOWNERS = ["@yourgithubusername"]  # Gib hier deinen GitHub-Namen an

# Namespace erstellen
my_tls_mqtt_ns = cg.esphome_ns.namespace("my_tls_mqtt")
MyTLSMQTTClient = my_tls_mqtt_ns.class_("MyTLSMQTTClient", cg.Component)

# Konfigurationsschema definieren
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MyTLSMQTTClient),
    cv.Required(CONF_HOST): cv.string,  # Broker-Host
    cv.Required(CONF_PORT): cv.port,    # Broker-Port
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_host(config[CONF_HOST], config[CONF_PORT]))
