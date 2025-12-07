## File: docs/wiring.md

```markdown
# Wiring Guide

## Overview
This guide provides detailed wiring instructions for connecting the AD9833 function generator with MCP41010 amplitude control and ST7796 touchscreen display to an ESP32.

## Required Components

### Main Components
- ESP32 Development Board (38-pin recommended)
- AD9833 Waveform Generator Module
- MCP41010 Digital Potentiometer (DIP-8 or SOIC-8)
- ST7796 TFT LCD Display (480x320 with XPT2046 touch)

### Additional Components
- Breadboard or PCB
- Jumper wires
- 10kΩ resistor (optional, for output buffering)
- Decoupling capacitors (0.1µF ceramic)

## Pin Mapping Table

| ESP32 Pin | Function | AD9833 | MCP41010 | ST7796 | XPT2046 |
|-----------|----------|--------|----------|--------|---------|
| GPIO18    | SPI CLK  | SCLK   | SCK      | SCK    | T_CLK   |
| GPIO23    | SPI MOSI | SDATA  | SI       | SDA    | T_DIN   |
| GPIO19    | SPI MISO | -      | -        | SDO    | T_DO    |
| GPIO5     | CS1      | FSYNC  | -        | -      | -       |
| GPIO15    | CS2      | -      | CS       | -      | -       |
| GPIO14    | CS3      | -      | -        | CS     | -       |
| GPIO12    | CS4      | -      | -        | -      | T_CS    |
| GPIO27    | DC       | -      | -        | DC/RS  | -       |
| GPIO33    | RESET    | -      | -        | RESET  | -       |
| GPIO35    | IRQ      | -      | -        | -      | T_IRQ   |
| 3.3V      | Power    | VCC    | VDD      | VCC    | VCC     |
| GND       | Ground   | GND    | GND/VSS  | GND    | GND     |

## Detailed Wiring

### 1. SPI Bus (Shared)
Connect the shared SPI bus to all devices:

```
ESP32 GPIO18 (CLK)  ──┬─── AD9833 SCLK
                      ├─── MCP41010 SCK
                      ├─── ST7796 SCK
                      └─── XPT2046 T_CLK

ESP32 GPIO23 (MOSI) ──┬─── AD9833 SDATA
                      ├─── MCP41010 SI
                      ├─── ST7796 SDA
                      └─── XPT2046 T_DIN

ESP32 GPIO19 (MISO) ──┬─── ST7796 SDO (optional)
                      └─── XPT2046 T_DO
```

### 2. AD9833 Waveform Generator

```
AD9833 Pin    ESP32 Pin    Function
──────────────────────────────────────
1  COMP       -            Not used
2  VCC        3.3V         Power
3  GND        GND          Ground
4  MCLK       -            Crystal (internal)
5  DGND       GND          Digital ground
6  VOUT       MCP41010     Signal output
7  AGND       GND          Analog ground
8  SDATA      GPIO23       SPI MOSI
9  SCLK       GPIO18       SPI CLK
10 FSYNC      GPIO5        Chip select
```

**Important Notes:**
- AD9833 operates at 2.3V-5.5V (3.3V recommended)
- FSYNC (chip select) is active LOW
- 25 MHz crystal typically included on module
- Connect AGND and DGND together to ESP32 GND

### 3. MCP41010 Digital Potentiometer

The MCP41010 acts as a voltage divider for amplitude control:

```
MCP41010 Pin  Connection      Function
────────────────────────────────────────
1  CS         GPIO15          Chip select
2  SCK        GPIO18          SPI clock
3  SI         GPIO23          SPI data in
4  VSS        GND             Ground
5  PA0        AD9833 VOUT     Top of pot (input)
6  PW0        Output signal   Wiper (variable output)
7  PB0        GND             Bottom of pot (ground)
8  VDD        3.3V            Power
```

**Wiring for Amplitude Control:**
```
AD9833 VOUT ──→ MCP41010 PA0 (pin 5)
                MCP41010 PW0 (pin 6) ──→ Output Signal
                MCP41010 PB0 (pin 7) ──→ GND
```

### 4. ST7796 TFT Display

```
ST7796 Pin    ESP32 Pin    Function
──────────────────────────────────────
VCC           3.3V         Power (or 5V if compatible)
GND           GND          Ground
CS            GPIO14       Chip select
RESET         GPIO33       Reset
DC/RS         GPIO27       Data/Command
SDA/MOSI      GPIO23       SPI MOSI
SCK           GPIO18       SPI CLK
LED           3.3V         Backlight (via resistor)
SDO/MISO      GPIO19       SPI MISO (optional)
```

**Backlight Connection:**
- Connect LED pin through a 100Ω resistor to 3.3V for full brightness
- Or connect to a PWM-capable GPIO for brightness control

### 5. XPT2046 Touch Controller

```
XPT2046 Pin   ESP32 Pin    Function
──────────────────────────────────────
VCC           3.3V         Power
GND           GND          Ground
CS            GPIO12       Chip select
IRQ           GPIO35       Interrupt (input only)
DIN           GPIO23       SPI MOSI
CLK           GPIO18       SPI CLK
DO            GPIO19       SPI MISO
```

**Important Notes:**
- GPIO35 is input-only on ESP32, perfect for IRQ
- IRQ is active LOW (triggers on touch)
- Touch controller shares SPI bus with display

## Power Supply Recommendations

### Power Requirements
- ESP32: 500mA typical, 800mA peak
- ST7796 Display: 100-200mA (depends on brightness)
- AD9833: 10-15mA
- MCP41010: 1mA
- **Total**: ~800mA typical, 1A recommended

### Power Supply Options
1. **USB Power**: 5V USB → ESP32 onboard regulator → 3.3V
2. **External 5V**: Connect to ESP32 5V pin
3. **Dedicated 3.3V**: Connect to 3.3V pin (bypass regulator)

### Decoupling Capacitors
Place 0.1µF ceramic capacitors close to:
- AD9833 VCC to GND
- MCP41010 VDD to VSS
- ST7796 VCC to GND
- ESP32 3.3V to GND

## Output Circuit

### Basic Output
```
MCP41010 PW0 ──→ Output BNC/Terminal
```

### Buffered Output (Recommended)
```
MCP41010 PW0 ──→ Op-Amp Buffer ──→ Output
                 (e.g., TL072)
```

### With Load Protection
```
MCP41010 PW0 ──┬─── 1kΩ resistor ──→ Output
               └─── 10kΩ to GND (pull-down)
```

## Troubleshooting

### No Display
- Check power supply (3.3V or 5V)
- Verify CS, DC, and RESET pins
- Test SPI communication with logic analyzer
- Try different rotation settings in code

### Touch Not Working
- Verify IRQ pin connection (GPIO35)
- Check touch calibration values
- Test with lower threshold
- Ensure MISO connected for touch

### No Signal Output
- Verify AD9833 power and crystal
- Check SPI connections
- Measure VOUT with oscilloscope
- Test without MCP41010 (direct AD9833 output)

### Amplitude Control Issues
- Verify MCP41010 wiring as voltage divider
- Check CS pin for digipot
- Test with known wiper values
- Measure resistance across PA0-PB0

## Testing Procedure

1. **Power Test**: Verify 3.3V on all VCC pins
2. **SPI Test**: Use logic analyzer to verify clock and data
3. **AD9833 Test**: Check for 1kHz sine wave on VOUT
4. **Display Test**: Verify display shows content
5. **Touch Test**: Monitor touch coordinates in logs
6. **Amplitude Test**: Verify MCP41010 changes output level

## Safety Notes

- Never exceed 3.6V on ESP32 GPIO pins
- Use appropriate current limiting for LED backlight
- Avoid connecting inductive loads directly to outputs
- Use ESD protection when handling components
- Double-check polarity before applying power

## Additional Resources

- [AD9833 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/AD9833.pdf)
- [MCP41010 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/11195c.pdf)
- [ESP32 Pinout Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [XPT2046 Touch Controller Info](https://github.com/PaulStoffregen/XPT2046_Touchscreen)
```

---

