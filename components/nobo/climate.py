import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, i2c, sensor
from esphome.const import (
    CONF_ID,
    CONF_POWER,
    UNIT_WATT,
    DEVICE_CLASS_POWER,
    STATE_CLASS_MEASUREMENT,
    CONF_UPDATE_INTERVAL,
)

CODEOWNERS = ["@CsabaHegedus"]
DEPENDENCIES = ['i2c']
AUTO_LOAD = ["sensor"]

CONF_I2C_ADDR = 0x01
CONF_NOMINAL_POWER = 'nominal_power'
power = cv.float_with_unit("power", "(w|W|watt|Watts)?")

nobo_ns = cg.esphome_ns.namespace("nobo")
NoboClimate = nobo_ns.class_("NoboClimate", climate.Climate, cg.Component, i2c.I2CDevice)
NoboClimateTargetTempConfig = nobo_ns.struct("NoboClimateTargetTempConfig")

CONFIG_SCHEMA = cv.All(
    climate.CLIMATE_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(NoboClimate),
            cv.Required(CONF_NOMINAL_POWER): cv.All(
                power, cv.Range(min=200.0, max=2500.0)
            ),
            cv.Optional(CONF_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(
                CONF_UPDATE_INTERVAL, "30s"
            ): cv.positive_time_period_seconds,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(CONF_I2C_ADDR)),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_nominal_power_watt(config[CONF_NOMINAL_POWER]))
    cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))

    if CONF_POWER in config :
        sens = await sensor.new_sensor(config[CONF_POWER])
        cg.add(var.set_power_sensor(sens))