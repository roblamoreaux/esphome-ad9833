import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import spi
from esphome.const import CONF_ID
from esphome import automation, pins

DEPENDENCIES = ["spi"]
AUTO_LOAD = ["sensor"]

ad9833_ns = cg.esphome_ns.namespace("ad9833")
AD9833 = ad9833_ns.class_("AD9833", cg.Component, spi.SPIDevice)

WaveformType = ad9833_ns.enum("WaveformType")
WAVEFORMS = {
    "SINE": WaveformType.WAVEFORM_SINE,
    "TRIANGLE": WaveformType.WAVEFORM_TRIANGLE,
    "SQUARE": WaveformType.WAVEFORM_SQUARE,
}

SweepType = ad9833_ns.enum("SweepType")
SWEEP_TYPES = {
    "LINEAR": SweepType.SWEEP_LINEAR,
    "LOGARITHMIC": SweepType.SWEEP_LOGARITHMIC,
}

ModulationType = ad9833_ns.enum("ModulationType")
MODULATION_TYPES = {
    "NONE": ModulationType.MOD_NONE,
    "AM": ModulationType.MOD_AM,
    "FM": ModulationType.MOD_FM,
    "FSK": ModulationType.MOD_FSK,
}

CONF_FREQUENCY = "frequency"
CONF_WAVEFORM = "waveform"
CONF_PHASE = "phase"
CONF_DIGIPOT_CS_PIN = "digipot_cs_pin"
CONF_AMPLITUDE = "amplitude"
CONF_START_FREQ = "start_frequency"
CONF_END_FREQ = "end_frequency"
CONF_DURATION = "duration"
CONF_SWEEP_TYPE = "sweep_type"
CONF_MOD_TYPE = "modulation_type"
CONF_MOD_FREQ = "modulation_frequency"
CONF_MOD_DEPTH = "modulation_depth"
CONF_FSK_FREQ1 = "fsk_frequency_1"
CONF_FSK_FREQ2 = "fsk_frequency_2"
CONF_FSK_STATE = "fsk_state"

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(): cv.declare_id(AD9833),
        cv.Optional(CONF_FREQUENCY, default=1000.0): cv.float_range(min=0.0, max=12500000.0),
        cv.Optional(CONF_WAVEFORM, default="SINE"): cv.enum(WAVEFORMS, upper=True),
        cv.Optional(CONF_PHASE, default=0.0): cv.float_range(min=0.0, max=360.0),
        cv.Optional(CONF_DIGIPOT_CS_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_AMPLITUDE, default=50.0): cv.float_range(min=0.0, max=100.0),
    })
    .extend(cv.COMPONENT_SCHEMA)
    .extend(spi.spi_device_schema(cs_pin_required=True))
)

# Actions
SetAmplitudeAction = ad9833_ns.class_("SetAmplitudeAction", automation.Action)
StartSweepAction = ad9833_ns.class_("StartSweepAction", automation.Action)
StopSweepAction = ad9833_ns.class_("StopSweepAction", automation.Action)
SetModulationAction = ad9833_ns.class_("SetModulationAction", automation.Action)
StopModulationAction = ad9833_ns.class_("StopModulationAction", automation.Action)
SetFSKFrequenciesAction = ad9833_ns.class_("SetFSKFrequenciesAction", automation.Action)
SetFSKStateAction = ad9833_ns.class_("SetFSKStateAction", automation.Action)

@automation.register_action(
    "ad9833.set_amplitude",
    SetAmplitudeAction,
    cv.Schema({
        cv.GenerateID(): cv.use_id(AD9833),
        cv.Required(CONF_AMPLITUDE): cv.templatable(cv.float_range(min=0.0, max=100.0)),
    })
)
async def set_amplitude_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

@automation.register_action(
    "ad9833.start_sweep",
    StartSweepAction,
    cv.Schema({
        cv.GenerateID(): cv.use_id(AD9833),
        cv.Required(CONF_START_FREQ): cv.templatable(cv.float_),
        cv.Required(CONF_END_FREQ): cv.templatable(cv.float_),
        cv.Required(CONF_DURATION): cv.templatable(cv.positive_float),
        cv.Optional(CONF_SWEEP_TYPE, default="LINEAR"): cv.enum(SWEEP_TYPES, upper=True),
    })
)
async def start_sweep_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

@automation.register_action(
    "ad9833.stop_sweep",
    StopSweepAction,
    cv.Schema({
        cv.GenerateID(): cv.use_id(AD9833),
    })
)
async def stop_sweep_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

@automation.register_action(
    "ad9833.set_modulation",
    SetModulationAction,
    cv.Schema({
        cv.GenerateID(): cv.use_id(AD9833),
        cv.Required(CONF_MOD_TYPE): cv.enum(MODULATION_TYPES, upper=True),
        cv.Required(CONF_MOD_FREQ): cv.templatable(cv.float_),
        cv.Required(CONF_MOD_DEPTH): cv.templatable(cv.float_range(min=0.0, max=1.0)),
    })
)
async def set_modulation_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await spi.register_spi_device(var, config)
    
    # Setup digital potentiometer if configured
    if CONF_DIGIPOT_CS_PIN in config:
        digipot_cs = await cg.gpio_pin_expression(config[CONF_DIGIPOT_CS_PIN])
        cg.add(var.set_digipot_cs_pin(digipot_cs))

# Example usage in ESPHome YAML:
# external_components:
#   - source: github://yourusername/esphome-ad9833
#     components: [ ad9833 ]
#
# spi:
#   clk_pin: GPIO18
#   mosi_pin: GPIO23
#
# ad9833:
#   id: my_ad9833
#   cs_pin: GPIO5
#   digipot_cs_pin: GPIO15  # MCP41010 CS pin
#   frequency: 1000.0
#   waveform: SINE
#   amplitude: 50.0  # Start at 50%
#
# number:
#   - platform: template
#     name: "Signal Amplitude"
#     min_value: 0
#     max_value: 100
#     step: 1
#     optimistic: true
#     set_action:
#       - ad9833.set_amplitude:
#           amplitude: !lambda 'return x;'
#
# button:
#   - platform: template
#     name: "Start Linear Sweep"
#     on_press:
#       - ad9833.start_sweep:
#           start_frequency: 100.0
#           end_frequency: 10000.0
#           duration: 5000  # 5 seconds
#           sweep_type: LINEAR
#
#   - platform: template
#     name: "True AM Modulation"
#     on_press:
#       - ad9833.set_modulation:
#           modulation_type: AM
#           modulation_frequency: 10.0  # 10 Hz modulation
#           modulation_depth: 0.5  # 50% depth
#
#   - platform: template
#     name: "Set Low Amplitude"
#     on_press:
#       - ad9833.set_amplitude:
#           amplitude: 10.0
#
#   - platform: template
#     name: "Set High Amplitude"
#     on_press:
#       - ad9833.set_amplitude:
#           amplitude: 90.0