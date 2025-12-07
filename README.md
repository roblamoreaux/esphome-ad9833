![GitHub release](https://img.shields.io/github/v/release/roblamoreaux/esphome-ad9833)
![GitHub](https://img.shields.io/github/license/roblamoreaux/esphome-ad9833)
![GitHub stars](https://img.shields.io/github/stars/roblamoreaux/esphome-ad9833)
![GitHub issues](https://img.shields.io/github/issues/roblamoreaux/esphome-ad9833)
![ESPHome](https://img.shields.io/badge/ESPHome-2024.11+-green.svg)

# ESPHome AD9833 Function Generator

A complete ESPHome external component for the AD9833 programmable waveform generator with MCP41010 digital potentiometer amplitude control, featuring an advanced touchscreen interface for ESP32.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![ESPHome](https://img.shields.io/badge/ESPHome-2024.11+-green.svg)

## Features

### Core Functionality
- ✨ **Waveform Generation**: Sine, Triangle, and Square waves
- 📊 **Frequency Range**: 0 Hz to 12.5 MHz
- 🎚️ **Amplitude Control**: 0-100% via MCP41010 digital potentiometer
- 🌊 **Frequency Sweeps**: Linear and logarithmic sweep modes
- 📡 **Modulation**: AM, FM, and FSK modulation support
- ⚡ **Hardware FSK**: Fast frequency switching using dual registers
- 🎯 **Phase Control**: 0-360° phase adjustment

### Advanced Features
- 🖥️ **Professional UI**: ST7796 480x320 touchscreen interface
- 👆 **Resistive Touch**: XPT2046 touchscreen controller
- 📈 **Scope Mode**: Real-time waveform visualization
- 🎛️ **Interactive Controls**: Touch sliders and buttons
- 💾 **Preset System**: 6 built-in signal presets
- 📱 **Home Assistant**: Full integration with all controls
- 🔄 **Real-time Updates**: Live status bar and scope display

## Hardware Requirements

### Required Components
- **ESP32 Development Board**
- **AD9833 Waveform Generator Module** (25 MHz crystal)
- **MCP41010 Digital Potentiometer** (10kΩ, SPI)
- **ST7796 TFT Display** (480x320, SPI)
- **XPT2046 Touch Controller** (resistive touchscreen)

### Pin Connections

```
ESP32 Pin     Component
─────────────────────────────────────
GPIO18        SPI CLK (all devices)
GPIO23        SPI MOSI (all devices)
GPIO19        SPI MISO (touch only)

GPIO5         AD9833 CS
GPIO15        MCP41010 CS
GPIO14        ST7796 CS
GPIO27        ST7796 DC
GPIO33        ST7796 RST
GPIO12        XPT2046 CS
GPIO35        XPT2046 IRQ
```

### Circuit Diagram

```
AD9833 Output Signal Path:
┌─────────┐
│ AD9833  │
│  VOUT   ├──────┐
└─────────┘      │
                 ↓
            ┌─────────┐
            │MCP41010 │
            │   PA0   │ ← Top (from AD9833)
            │   PW0   ├─────→ Output Signal
            │   PB0   │ ← Bottom (GND)
            └─────────┘

All SPI devices share CLK and MOSI lines.
Each device has independent CS (chip select).
```

## Installation

### Method 1: Direct from GitHub (Recommended)

Add to your ESPHome YAML configuration:

```yaml
external_components:
  - source: github://roblamoreaux/esphome-ad9833
    components: [ ad9833 ]
```

### Method 2: Local Installation

1. Clone this repository:
```bash
git clone https://github.com/roblamoreaux/esphome-ad9833.git
```

2. Copy to your ESPHome configuration:
```bash
cp -r esphome-ad9833/components/ad9833 /config/esphome/components/
```

3. Reference in YAML:
```yaml
external_components:
  - source: components
    components: [ ad9833 ]
```

## Quick Start

### Basic Configuration

```yaml
# Minimal working example
spi:
  clk_pin: GPIO18
  mosi_pin: GPIO23

ad9833:
  id: sig_gen
  cs_pin: GPIO5
  frequency: 1000.0
  waveform: SINE
```

### With Amplitude Control

```yaml
ad9833:
  id: sig_gen
  cs_pin: GPIO5
  digipot_cs_pin: GPIO15  # Add MCP41010
  frequency: 1000.0
  waveform: SINE
  amplitude: 50.0
```

### Complete Touchscreen System

See [example-complete.yaml](examples/example-complete.yaml) for the full configuration with touchscreen interface.

## Usage Examples

### Frequency Control

```yaml
number:
  - platform: template
    name: "Frequency"
    min_value: 10
    max_value: 20000
    step: 1
    set_action:
      - lambda: id(sig_gen)->set_frequency(x);
```

### Waveform Selection

```yaml
button:
  - platform: template
    name: "Sine Wave"
    on_press:
      - lambda: id(sig_gen)->set_waveform(ad9833::WAVEFORM_SINE);

  - platform: template
    name: "Triangle Wave"
    on_press:
      - lambda: id(sig_gen)->set_waveform(ad9833::WAVEFORM_TRIANGLE);

  - platform: template
    name: "Square Wave"
    on_press:
      - lambda: id(sig_gen)->set_waveform(ad9833::WAVEFORM_SQUARE);
```

### Frequency Sweep

```yaml
button:
  - platform: template
    name: "Audio Sweep"
    on_press:
      - lambda: |-
          id(sig_gen)->start_sweep(
            20.0,      // Start: 20 Hz
            20000.0,   // End: 20 kHz
            10000.0,   // Duration: 10 seconds
            ad9833::SWEEP_LOGARITHMIC
          );

  - platform: template
    name: "Stop Sweep"
    on_press:
      - lambda: id(sig_gen)->stop_sweep();
```

### Modulation

```yaml
button:
  - platform: template
    name: "FM Modulation"
    on_press:
      - lambda: |-
          id(sig_gen)->set_modulation(
            ad9833::MOD_FM,    // Frequency modulation
            10.0,              // 10 Hz mod frequency
            0.1                // 10% deviation
          );

  - platform: template
    name: "AM Modulation"
    on_press:
      - lambda: |-
          id(sig_gen)->set_modulation(
            ad9833::MOD_AM,    // Amplitude modulation
            5.0,               // 5 Hz mod frequency
            0.5                // 50% depth
          );
```

### FSK (Frequency Shift Keying)

```yaml
button:
  - platform: template
    name: "Setup FSK"
    on_press:
      - lambda: |-
          id(sig_gen)->set_fsk_frequencies(1200.0, 2200.0);

  - platform: template
    name: "FSK Mark"
    on_press:
      - lambda: id(sig_gen)->set_fsk_state(false);

  - platform: template
    name: "FSK Space"
    on_press:
      - lambda: id(sig_gen)->set_fsk_state(true);
```

### Amplitude Control

```yaml
number:
  - platform: template
    name: "Signal Amplitude"
    min_value: 0
    max_value: 100
    step: 1
    unit_of_measurement: "%"
    set_action:
      - lambda: id(sig_gen)->set_amplitude(x);
```

## API Reference

### Methods

#### Basic Control
- `set_frequency(float frequency)` - Set output frequency (0-12.5 MHz)
- `set_waveform(WaveformType waveform)` - Set waveform type
- `set_phase(float phase_deg)` - Set phase offset (0-360°)
- `set_amplitude(float percent)` - Set amplitude (0-100%, requires MCP41010)
- `sleep(bool enable)` - Enable/disable sleep mode
- `reset()` - Reset the AD9833

#### Sweep Control
- `start_sweep(float start_freq, float end_freq, float duration_ms, SweepType type)` - Start frequency sweep
- `stop_sweep()` - Stop active sweep
- `is_sweeping()` - Check if sweep is active

#### Modulation Control
- `set_modulation(ModulationType type, float mod_freq, float depth)` - Configure modulation
- `stop_modulation()` - Stop modulation
- `is_modulating()` - Check if modulation is active

#### FSK Control
- `set_fsk_frequencies(float freq1, float freq2)` - Configure FSK frequencies
- `set_fsk_state(bool state)` - Switch FSK state (false=freq1, true=freq2)

### Enumerations

#### WaveformType
- `WAVEFORM_SINE` - Sine wave
- `WAVEFORM_TRIANGLE` - Triangle wave
- `WAVEFORM_SQUARE` - Square wave

#### SweepType
- `SWEEP_LINEAR` - Linear frequency sweep
- `SWEEP_LOGARITHMIC` - Logarithmic frequency sweep

#### ModulationType
- `MOD_NONE` - No modulation
- `MOD_AM` - Amplitude modulation
- `MOD_FM` - Frequency modulation
- `MOD_FSK` - Frequency shift keying

## Touchscreen Interface

The complete example includes a professional touchscreen interface with three pages:

### Page 1: Controls
- Interactive frequency slider (10 Hz - 20 kHz)
- Interactive amplitude slider (0-100%)
- Large numeric displays
- Real-time value updates

### Page 2: Presets
- **1kHz Sine**: Standard test signal
- **440Hz Audio**: Musical A note
- **Audio Sweep**: 20 Hz - 20 kHz logarithmic sweep
- **RF Test**: 10 kHz square wave
- **FSK Demo**: 1200/2200 Hz frequency shift keying
- **AM Radio**: 1 kHz with amplitude modulation

### Page 3: Scope Mode
- Real-time waveform visualization
- 2-cycle waveform display
- Sweep progress indicator
- Technical information overlay
- Period and wavelength calculations

## Calibration

### Touchscreen Calibration

The touchscreen may require calibration for your specific display. Adjust these values in the YAML:

```yaml
touchscreen:
  - platform: xpt2046
    calibration_x_min: 280
    calibration_x_max: 3860
    calibration_y_min: 340
    calibration_y_max: 3860
    swap_x_y: false
```

To calibrate:
1. Enable touch coordinate logging in ESPHome
2. Touch each corner of the screen
3. Note the raw coordinates
4. Update the calibration values

### Frequency Calibration

If your AD9833 module has a different crystal frequency (default is 25 MHz), update in `ad9833.cpp`:

```cpp
const float FMCLK = 25000000.0; // Change to your crystal frequency
```

## Troubleshooting

### No Signal Output
- Verify SPI connections (CLK, MOSI, CS)
- Check AD9833 power supply (2.3-5.5V)
- Ensure crystal oscillator is functioning
- Verify chip select pin is correct

### Amplitude Control Not Working
- Check MCP41010 connections
- Verify digipot_cs_pin is configured
- Ensure MCP41010 is wired as voltage divider
- Test with known good potentiometer values

### Touchscreen Not Responding
- Verify XPT2046 CS and IRQ pins
- Check touchscreen calibration values
- Test with lower threshold value
- Ensure SPI MISO is connected for touch

### Display Issues
- Verify ST7796 DC and RST pins
- Check display power supply
- Try different rotation settings
- Ensure font files are accessible

### Sweep Not Working
- Check that loop() method is being called
- Verify sweep parameters are valid
- Ensure sweep_active is set correctly
- Monitor logs for frequency updates

## Applications

- 🔬 **Electronics Testing**: Signal injection and circuit testing
- 🎵 **Audio Testing**: Speaker and amplifier characterization
- 📡 **RF Development**: Carrier generation and modulation testing
- 🎓 **Education**: Waveform demonstration and learning
- 🔊 **Audio Synthesis**: Tone generation and effects
- 📞 **Communications**: FSK modem testing
- 🎛️ **Function Generator**: General-purpose signal generation

## Performance

- **Frequency Resolution**: 0.1 Hz (28-bit accumulator)
- **Frequency Accuracy**: Crystal dependent (±100 ppm typical)
- **Phase Resolution**: 0.088° (12-bit accumulator)
- **Amplitude Resolution**: 256 steps (8-bit digipot)
- **Update Rate**: ~100 Hz (display refresh)
- **Sweep Rate**: Millisecond resolution
- **Modulation Rate**: Up to 100 Hz typical

## Project Structure

```
esphome-ad9833/
├── components/
│   └── ad9833/
│       ├── __init__.py          # Component registration
│       ├── ad9833.h             # Header file
│       └── ad9833.cpp           # Implementation
├── examples/
│   ├── basic.yaml               # Minimal example
│   ├── amplitude-control.yaml   # With MCP41010
│   └── complete.yaml            # Full touchscreen system
├── docs/
│   ├── wiring.md               # Detailed wiring guide
│   └── calibration.md          # Calibration procedures
├── README.md                    # This file
└── LICENSE                      # MIT License
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

### Development Setup

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Credits

- **AD9833**: Analog Devices - Programmable Waveform Generator
- **MCP41010**: Microchip - 10kΩ Digital Potentiometer
- **ESPHome**: Home Assistant community project
- **ST7796**: Sitronix - TFT LCD Controller
- **XPT2046**: Texas Instruments - Resistive Touch Controller

## Support

- 📖 [Documentation](https://github.com/roblamoreaux/esphome-ad9833/wiki)
- 🐛 [Issue Tracker](https://github.com/roblamoreaux/esphome-ad9833/issues)
- 💬 [Discussions](https://github.com/roblamoreaux/esphome-ad9833/discussions)
- 🏠 [ESPHome Community](https://community.home-assistant.io/)

## Changelog

### v1.0.0 (2024-12-07)
- Initial release
- AD9833 waveform generation support
- MCP41010 amplitude control
- Linear and logarithmic frequency sweeps
- AM, FM, and FSK modulation
- ST7796 touchscreen interface
- Real-time scope visualization
- Preset system
- Home Assistant integration

## Roadmap

- [ ] Add frequency counter input
- [ ] Implement burst mode
- [ ] Add arbitrary waveform support
- [ ] Phase-locked loop (PLL) features
- [ ] Save/load user presets
- [ ] Network-based control API
- [ ] Oscilloscope mode with sampling
- [ ] Multiple output channels

## Acknowledgments

Special thanks to the ESPHome and Home Assistant communities for their excellent documentation and support.

---

**Made with ❤️ for the maker community**

If you find this project useful, please consider giving it a ⭐ on GitHub!