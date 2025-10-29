"""IWR6843 Number Platform"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import (
    CONF_ID,
    CONF_MODE,
    UNIT_CENTIMETER,
    UNIT_METER,
)
from . import IWR6843Component, CONF_IWR6843_ID, iwr6843_ns

DEPENDENCIES = ["iwr6843"]

CONF_NUMBER_TYPE = "number_type"
CONF_BOUNDARY_TYPE = "boundary_type"

IWR6843Number = iwr6843_ns.class_("IWR6843Number", number.Number, cg.Component)

NumberType = iwr6843_ns.enum("NumberType")
NUMBER_TYPES = {
    "ceiling_height": NumberType.CEILING_HEIGHT,
    "max_tracks": NumberType.MAX_TRACKS,
    "tracking_boundary_x_max": NumberType.TRACKING_BOUNDARY_X_MAX,
    "tracking_boundary_x_min": NumberType.TRACKING_BOUNDARY_X_MIN,
    "tracking_boundary_y_max": NumberType.TRACKING_BOUNDARY_Y_MAX,
    "tracking_boundary_y_min": NumberType.TRACKING_BOUNDARY_Y_MIN,
    "tracking_boundary_z_max": NumberType.TRACKING_BOUNDARY_Z_MAX,
    "tracking_boundary_z_min": NumberType.TRACKING_BOUNDARY_Z_MIN,
    "presence_boundary_x_max": NumberType.PRESENCE_BOUNDARY_X_MAX,
    "presence_boundary_x_min": NumberType.PRESENCE_BOUNDARY_X_MIN,
    "presence_boundary_y_max": NumberType.PRESENCE_BOUNDARY_Y_MAX,
    "presence_boundary_y_min": NumberType.PRESENCE_BOUNDARY_Y_MIN,
    "presence_boundary_z_max": NumberType.PRESENCE_BOUNDARY_Z_MAX,
    "presence_boundary_z_min": NumberType.PRESENCE_BOUNDARY_Z_MIN,
}

CONFIG_SCHEMA = number.number_schema(IWR6843Number, icon="mdi:ruler").extend(
    {
        cv.GenerateID(CONF_IWR6843_ID): cv.use_id(IWR6843Component),
        cv.Required(CONF_NUMBER_TYPE): cv.enum(NUMBER_TYPES, lower=True),
    }
)


async def to_code(config):
    """Generate number code"""
    parent = await cg.get_variable(config[CONF_IWR6843_ID])
    num = cg.new_Pvariable(config[CONF_ID])
    await number.register_number(
        num,
        config,
        min_value=config.get("min_value", 0),
        max_value=config.get("max_value", 100),
        step=config.get("step", 1),
    )
    await cg.register_component(num, config)
    
    cg.add(num.set_parent(parent))
    cg.add(num.set_number_type(config[CONF_NUMBER_TYPE]))

