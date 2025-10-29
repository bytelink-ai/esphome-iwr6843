"""ESPHome IWR6843 mmWave Radar Component"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import spi, uart, sensor, binary_sensor, button, switch, number
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    DEVICE_CLASS_OCCUPANCY,
    DEVICE_CLASS_SAFETY,
    STATE_CLASS_MEASUREMENT,
    UNIT_CENTIMETER,
)

DEPENDENCIES = ["spi", "uart"]
AUTO_LOAD = ["sensor", "binary_sensor", "button", "switch", "number"]

CONF_IWR6843_ID = "iwr6843_id"
CONF_SOP2_PIN = "sop2_pin"
CONF_NRST_PIN = "nrst_pin"
CONF_CS_PIN = "cs_pin"
CONF_CEILING_HEIGHT = "ceiling_height"
CONF_MAX_TRACKS = "max_tracks"
CONF_TRACKING_BOUNDARY = "tracking_boundary"
CONF_PRESENCE_BOUNDARY = "presence_boundary"
CONF_TRACKING_IDS = "tracking_ids"
CONF_X_MAX = "x_max"
CONF_X_MIN = "x_min"
CONF_Y_MAX = "y_max"
CONF_Y_MIN = "y_min"
CONF_Z_MAX = "z_max"
CONF_Z_MIN = "z_min"

iwr6843_ns = cg.esphome_ns.namespace("iwr6843")
IWR6843Component = iwr6843_ns.class_(
    "IWR6843Component", cg.Component, spi.SPIDevice, uart.UARTDevice
)

# Tracking ID Schema
TRACKING_ID_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ID): cv.int_range(min=1, max=5),
        cv.Optional(CONF_NAME): cv.string,
    }
)

# Boundary Schema
BOUNDARY_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_X_MAX, default=4.0): cv.float_range(min=-10.0, max=10.0),
        cv.Optional(CONF_X_MIN, default=-4.0): cv.float_range(min=-10.0, max=10.0),
        cv.Optional(CONF_Y_MAX, default=4.0): cv.float_range(min=-10.0, max=10.0),
        cv.Optional(CONF_Y_MIN, default=-4.0): cv.float_range(min=-10.0, max=10.0),
        cv.Optional(CONF_Z_MAX, default=3.0): cv.float_range(min=-5.0, max=10.0),
        cv.Optional(CONF_Z_MIN, default=-0.5): cv.float_range(min=-5.0, max=10.0),
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(IWR6843Component),
            cv.Required(CONF_CS_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_SOP2_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_NRST_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_CEILING_HEIGHT, default=290): cv.int_range(
                min=100, max=500
            ),
            cv.Optional(CONF_MAX_TRACKS, default=5): cv.int_range(min=1, max=5),
            cv.Optional(CONF_TRACKING_BOUNDARY, default={}): BOUNDARY_SCHEMA,
            cv.Optional(CONF_PRESENCE_BOUNDARY, default={}): BOUNDARY_SCHEMA,
            cv.Optional(CONF_TRACKING_IDS, default=[]): cv.ensure_list(
                TRACKING_ID_SCHEMA
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(spi.spi_device_schema(cs_pin_required=False))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    """Generate C++ code from config"""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await spi.register_spi_device(var, config)
    await uart.register_uart_device(var, config)

    # Setup pins
    cs_pin = await cg.gpio_pin_expression(config[CONF_CS_PIN])
    cg.add(var.set_cs_pin(cs_pin))

    sop2_pin = await cg.gpio_pin_expression(config[CONF_SOP2_PIN])
    cg.add(var.set_sop2_pin(sop2_pin))

    nrst_pin = await cg.gpio_pin_expression(config[CONF_NRST_PIN])
    cg.add(var.set_nrst_pin(nrst_pin))

    # Setup configuration
    cg.add(var.set_ceiling_height(config[CONF_CEILING_HEIGHT]))
    cg.add(var.set_max_tracks(config[CONF_MAX_TRACKS]))

    # Tracking boundaries
    tracking = config[CONF_TRACKING_BOUNDARY]
    cg.add(
        var.set_tracking_boundary(
            tracking[CONF_X_MIN],
            tracking[CONF_X_MAX],
            tracking[CONF_Y_MIN],
            tracking[CONF_Y_MAX],
            tracking[CONF_Z_MIN],
            tracking[CONF_Z_MAX],
        )
    )

    # Presence boundaries
    presence = config[CONF_PRESENCE_BOUNDARY]
    cg.add(
        var.set_presence_boundary(
            presence[CONF_X_MIN],
            presence[CONF_X_MAX],
            presence[CONF_Y_MIN],
            presence[CONF_Y_MAX],
            presence[CONF_Z_MIN],
            presence[CONF_Z_MAX],
        )
    )

    # Setup tracking IDs (default to all 5 if not specified)
    tracking_ids = config[CONF_TRACKING_IDS]
    if not tracking_ids:
        tracking_ids = [{"id": i, "name": f"Person {i}"} for i in range(1, 6)]

    for track_config in tracking_ids:
        track_id = track_config[CONF_ID]
        track_name = track_config.get(CONF_NAME, f"Person {track_id}")
        cg.add(var.add_tracking_id(track_id, track_name))

