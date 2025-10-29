"""IWR6843 Switch Platform"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import (
    CONF_ID,
    ICON_FLASH,
)
from . import IWR6843Component, CONF_IWR6843_ID, iwr6843_ns

DEPENDENCIES = ["iwr6843"]

IWR6843FlashSwitch = iwr6843_ns.class_("IWR6843FlashSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = switch.switch_schema(IWR6843FlashSwitch, icon=ICON_FLASH).extend(
    {
        cv.GenerateID(CONF_IWR6843_ID): cv.use_id(IWR6843Component),
    }
)


async def to_code(config):
    """Generate switch code"""
    parent = await cg.get_variable(config[CONF_IWR6843_ID])
    sw = await switch.new_switch(config)
    await cg.register_component(sw, config)
    cg.add(sw.set_parent(parent))

