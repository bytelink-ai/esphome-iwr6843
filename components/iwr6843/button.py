"""IWR6843 Button Platform"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import (
    CONF_ID,
    ICON_RESTART,
)
from . import IWR6843Component, CONF_IWR6843_ID, iwr6843_ns

DEPENDENCIES = ["iwr6843"]

IWR6843ResetButton = iwr6843_ns.class_("IWR6843ResetButton", button.Button, cg.Component)

CONFIG_SCHEMA = button.button_schema(IWR6843ResetButton, icon=ICON_RESTART).extend(
    {
        cv.GenerateID(CONF_IWR6843_ID): cv.use_id(IWR6843Component),
    }
)


async def to_code(config):
    """Generate button code"""
    parent = await cg.get_variable(config[CONF_IWR6843_ID])
    btn = await button.new_button(config)
    await cg.register_component(btn, config)
    cg.add(btn.set_parent(parent))

