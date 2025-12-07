## File: fonts/README.md
# Fonts Directory

This directory should contain the TrueType font files used by the display.

## Required Fonts

Place your font files here:
- `Arial.ttf` (or any TrueType font)

## Font Sources

### Free Fonts
1. **Liberation Sans** (Arial alternative)
   - https://github.com/liberationfonts/liberation-fonts
   - Open source, metrically compatible with Arial

2. **Roboto**
   - https://fonts.google.com/specimen/Roboto
   - Clean, modern sans-serif font

3. **Ubuntu**
   - https://fonts.google.com/specimen/Ubuntu
   - Clear, readable UI font

### Installing Fonts

#### Linux
```bash
cp your-font.ttf /config/esphome/fonts/
```

#### Windows
Copy font file to your ESPHome config directory under `fonts/`

#### From Google Fonts
```bash
# Download font
wget https://github.com/google/roboto/releases/download/v2.138/roboto-unhinted.zip
unzip roboto-unhinted.zip
cp Roboto-Regular.ttf /config/esphome/fonts/Arial.ttf
```

## Using Different Fonts

Edit the font configuration in your YAML file:

```yaml
font:
  - file: "fonts/YourFont.ttf"
    id: font_small
    size: 14
  - file: "fonts/YourFont.ttf"
    id: font_medium
    size: 20
  - file: "fonts/YourFont.ttf"
    id: font_large
    size: 32
```

## Font Licenses

Ensure you comply with font licenses:
- **SIL Open Font License (OFL)**: Free for personal and commercial use
- **Apache License**: Free with attribution
- **GPL**: Free but requires sharing modifications

**Note**: Arial is a commercial font by Microsoft. Use Liberation Sans or Roboto as free alternatives.

