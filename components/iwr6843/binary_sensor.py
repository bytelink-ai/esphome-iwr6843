"""IWR6843 Binary Sensor Platform"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_OCCUPANCY,
    DEVICE_CLASS_SAFETY,
    ICON_ACCOUNT,
    ICON_ALERT_CIRCLE,
)
from . import IWR6843Component, CONF_IWR6843_ID

DEPENDENCIES = ["iwr6843"]

CONF_PERSON_ID = "person_id"
CONF_SENSOR_TYPE = "sensor_type"

SENSOR_TYPES = {
    "presence": DEVICE_CLASS_OCCUPANCY,
    "fall": DEVICE_CLASS_SAFETY,
}

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema().extend(
    {
        cv.GenerateID(CONF_IWR6843_ID): cv.use_id(IWR6843Component),
        cv.Required(CONF_PERSON_ID): cv.int_range(min=1, max=5),
        cv.Required(CONF_SENSOR_TYPE): cv.enum(SENSOR_TYPES, lower=True),
    }
)


async def to_code(config):
    """Generate binary sensor code"""
    parent = await cg.get_variable(config[CONF_IWR6843_ID])
    sens = await binary_sensor.new_binary_sensor(config)
    
    person_id = config[CONF_PERSON_ID]
    sensor_type = config[CONF_SENSOR_TYPE]
    
    if sensor_type == "presence":
        cg.add(parent.register_presence_sensor(person_id, sens))
    elif sensor_type == "fall":
        cg.add(parent.register_fall_sensor(person_id, sens))

