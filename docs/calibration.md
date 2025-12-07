## File: docs/calibration.md

```markdown
# Calibration Guide

## Touchscreen Calibration

### Why Calibration is Needed
Resistive touchscreens have manufacturing variations that affect touch coordinate mapping. Calibration ensures accurate touch detection across the entire screen.

### Calibration Process

#### Step 1: Enable Touch Logging
Add to your YAML:
```yaml
logger:
  level: DEBUG
  logs:
    touchscreen: DEBUG
```

#### Step 2: Find Raw Coordinates
1. Flash the configuration
2. Open ESPHome logs
3. Touch each corner of the screen
4. Record the raw coordinates:

```
Top-Left:     x_min, y_min
Top-Right:    x_max, y_min
Bottom-Left:  x_min, y_max
Bottom-Right: x_max, y_max
```

#### Step 3: Update Configuration
```yaml
touchscreen:
  - platform: xpt2046
    calibration_x_min: [your x_min]
    calibration_x_max: [your x_max]
    calibration_y_min: [your y_min]
    calibration_y_max: [your y_max]
    swap_x_y: false  # Set to true if axes are swapped
```

### Common Calibration Values

#### Portrait Mode (320x480)
```yaml
calibration_x_min: 280
calibration_x_max: 3860
calibration_y_min: 340
calibration_y_max: 3860
swap_x_y: false
```

#### Landscape Mode (480x320)
```yaml
calibration_x_min: 340
calibration_x_max: 3860
calibration_y_min: 280
calibration_y_max: 3860
swap_x_y: true
```

### Troubleshooting Touch Issues

**Touch Coordinates Inverted**
- Set `swap_x_y: true`

**Touch Y-axis Reversed**
- Swap `calibration_y_min` and `calibration_y_max`

**Touch X-axis Reversed**
- Swap `calibration_x_min` and `calibration_x_max`

**Erratic Touch Response**
- Increase `threshold` value (try 500-800)
- Check for electrical noise
- Add ferrite bead on touch wires

**No Touch Response**
- Verify IRQ pin connection
- Check MISO connection
- Test with `report_interval: 100ms`

## Frequency Calibration

### Crystal Accuracy
The AD9833 uses a 25 MHz crystal oscillator. Actual frequency depends on crystal accuracy (typically ±100 ppm).

### Measuring Crystal Frequency

#### Method 1: Frequency Counter
1. Generate a known frequency (e.g., 1 MHz)
2. Measure with frequency counter
3. Calculate error: `error = (measured - expected) / expected`
4. Update FMCLK in code

#### Method 2: Oscilloscope
1. Generate 1 kHz sine wave
2. Measure actual frequency with oscilloscope
3. Calculate correction factor
4. Update FMCLK value

### Updating Crystal Frequency

Edit `ad9833.cpp`:
```cpp
const float FMCLK = 25000000.0;  // Adjust this value
```

Calculate corrected value:
```
Corrected FMCLK = 25000000 * (measured_freq / expected_freq)
```

Example:
- Expected: 1000.0 Hz
- Measured: 1000.5 Hz
- Correction: `25000000 * (1000.5/1000) = 25012500`

### Frequency Accuracy Tips

- Use high-quality crystal oscillator module
- Keep crystal cool (avoid heat sources)
- Shield from RF interference
- Use proper decoupling capacitors
- Consider TCXO for <1 ppm accuracy

## Amplitude Calibration

### MCP41010 Linearity
The MCP41010 has typical ±20% tolerance. For accurate amplitude control:

#### Step 1: Measure Endpoints
1. Set amplitude to 0% - measure output voltage
2. Set amplitude to 100% - measure output voltage
3. Calculate voltage range

#### Step 2: Create Calibration Table
Measure output at multiple amplitude settings:
```
0%   → 0.00V
25%  → 0.23V
50%  → 0.47V
75%  → 0.71V
100% → 0.95V
```

#### Step 3: Apply Correction (Optional)
Implement linearization in code if needed:
```cpp
float calibrate_amplitude(float percent) {
    // Apply non-linear correction
    return percent + (correction_factor * percent);
}
```

### Output Level Adjustment

To change maximum output amplitude, adjust circuit:

**Increase Output:**
- Use op-amp buffer with gain
- Add output amplifier stage

**Decrease Output:**
- Add series resistor
- Use voltage divider after MCP41010

## Waveform Quality Optimization

### Reducing Distortion

**Hardware:**
- Add output buffer (op-amp)
- Use proper load impedance (>10kΩ)
- Add output filter capacitor (10nF-100nF)

**Software:**
- Limit maximum frequency to avoid aliasing
- Use appropriate waveform for application

### Measuring THD (Total Harmonic Distortion)

1. Generate 1 kHz sine wave
2. Measure with spectrum analyzer
3. Calculate THD from harmonics
4. Typical: <1% THD for sine wave

## Phase Calibration

Phase offset is typically accurate to ±0.1°. No calibration usually needed.

To verify phase accuracy:
1. Generate two outputs (if available)
2. Set known phase difference (e.g., 90°)
3. Measure with oscilloscope
4. Adjust phase register if needed

## Sweep Calibration

### Timing Accuracy
Sweep timing depends on ESP32 `millis()` function (typically ±1% accuracy).

To improve timing:
- Use hardware timer instead of millis()
- Implement interrupt-based updates
- Account for processing delays

### Frequency Step Accuracy
- Linear sweep: Steps determined by sample rate
- Logarithmic sweep: More steps at low frequencies
- Increase sweep duration for smoother output

## Validation Tests

### 1. Frequency Accuracy Test
```yaml
# Generate test frequencies
1 Hz    → Verify with long-term counter
100 Hz  → Check with oscilloscope
1 kHz   → Standard test frequency
10 kHz  → Check for distortion
100 kHz → Test high-frequency response
```

### 2. Amplitude Accuracy Test
```yaml
# Test amplitude levels
0%   → Should be minimum output
50%  → Should be half of maximum
100% → Should be maximum output
```

### 3. Waveform Quality Test
- Sine: Check for smooth curve, low THD
- Triangle: Verify linear rise/fall
- Square: Check for sharp edges, 50% duty cycle

### 4. Modulation Test
- AM: Verify depth corresponds to setting
- FM: Check deviation matches depth setting
- FSK: Verify frequency switching accuracy

## Calibration Checklist

- [ ] Touchscreen calibrated for accurate touch
- [ ] Crystal frequency measured and corrected
- [ ] Amplitude range verified
- [ ] Waveform quality checked
- [ ] Sweep timing validated
- [ ] Modulation depth verified
- [ ] Phase accuracy tested
- [ ] Documentation updated with values

## Recommended Equipment

- **Oscilloscope**: For waveform analysis
- **Frequency Counter**: For accuracy verification
- **Multimeter**: For voltage measurements
- **Spectrum Analyzer**: For THD measurements (optional)
- **Logic Analyzer**: For SPI debugging (optional)
```

---

