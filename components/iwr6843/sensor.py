"""IWR6843 Sensor Platform"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    STATE_CLASS_MEASUREMENT,
    UNIT_CENTIMETER,
)
from . import IWR6843Component, CONF_IWR6843_ID, iwr6843_ns

DEPENDENCIES = ["iwr6843"]

CONF_PERSON_ID = "person_id"
CONF_COORDINATE_TYPE = "coordinate_type"

CoordinateType = iwr6843_ns.enum("CoordinateType")
COORDINATE_TYPES = {
    "x": CoordinateType.X_COORDINATE,
    "y": CoordinateType.Y_COORDINATE,
    "z": CoordinateType.Z_COORDINATE,
}

CONFIG_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CENTIMETER,
    icon="mdi:ruler",
    accuracy_decimals=2,
    state_class=STATE_CLASS_MEASUREMENT,
).extend(
    {
        cv.GenerateID(CONF_IWR6843_ID): cv.use_id(IWR6843Component),
        cv.Required(CONF_PERSON_ID): cv.int_range(min=1, max=5),
        cv.Optional(CONF_COORDINATE_TYPE, default="x"): cv.enum(
            COORDINATE_TYPES, lower=True
        ),
    }
)


async def to_code(config):
    """Generate sensor code"""
    parent = await cg.get_variable(config[CONF_IWR6843_ID])
    sens = await sensor.new_sensor(config)
    
    person_id = config[CONF_PERSON_ID]
    coord_type = config[CONF_COORDINATE_TYPE]
    
    if coord_type == "x":
        cg.add(parent.register_x_coordinate_sensor(person_id, sens))
    elif coord_type == "y":
        cg.add(parent.register_y_coordinate_sensor(person_id, sens))
    elif coord_type == "z":
        cg.add(parent.register_z_coordinate_sensor(person_id, sens))

