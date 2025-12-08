// ad9833.cpp
#include "ad9833.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace ad9833 {

static const char *const TAG = "ad9833";

// AD9833 register bits
static const uint16_t REG_CONTROL = 0x0000;
static const uint16_t REG_FREQ0 = 0x4000;
static const uint16_t REG_FREQ1 = 0x8000;
static const uint16_t REG_PHASE0 = 0xC000;
static const uint16_t REG_PHASE1 = 0xE000;

// Control register bits
static const uint16_t CTRL_B28 = 0x2000;
static const uint16_t CTRL_HLB = 0x1000;
static const uint16_t CTRL_FSELECT = 0x0800;
static const uint16_t CTRL_PSELECT = 0x0400;
static const uint16_t CTRL_RESET = 0x0100;
static const uint16_t CTRL_SLEEP1 = 0x0080;
static const uint16_t CTRL_SLEEP12 = 0x0040;
static const uint16_t CTRL_OPBITEN = 0x0020;
static const uint16_t CTRL_DIV2 = 0x0008;
static const uint16_t CTRL_MODE = 0x0002;

// MCP41010 Implementation
MCP41010::MCP41010(spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH,
    spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_1MHZ> *parent, 
    GPIOPin *cs_pin) 
    : parent_(parent), cs_pin_(cs_pin) {}
//MCP41010::MCP41010(spi::SPIComponent *parent, GPIOPin *cs_pin) 
//    : parent_(parent), cs_pin_(cs_pin) {}
	
void MCP41010::setup() {
  this->cs_pin_->setup();
  this->cs_pin_->digital_write(true);
  
  // Set to mid-scale
  this->set_wiper(128);
}

void MCP41010::set_wiper(uint8_t value) {
  this->cs_pin_->digital_write(false);
  delayMicroseconds(1);
  
  this->parent_->write_byte(CMD_WRITE);
  this->parent_->write_byte(value);
  
  delayMicroseconds(1);
  this->cs_pin_->digital_write(true);
  
  this->current_wiper_ = value;
}

void MCP41010::set_amplitude(float percent) {
  if (percent < 0.0f) percent = 0.0f;
  if (percent > 100.0f) percent = 100.0f;
  
  uint8_t wiper = (uint8_t)((percent / 100.0f) * 255.0f);
  this->set_wiper(wiper);
}

// AD9833 Implementation
void AD9833::setup() {
  this->spi_setup();
  ESP_LOGCONFIG(TAG, "Setting up AD9833...");
  
  // Setup digital potentiometer if configured
  if (this->digipot_cs_pin_ != nullptr) {
    this->digipot_ = new MCP41010(this, this->digipot_cs_pin_);
    //this->digipot_ = new MCP41010(this->parent_, this->digipot_cs_pin_);
    this->digipot_->setup();
    this->mod_use_amplitude_ = true;
    ESP_LOGCONFIG(TAG, "MCP41010 digital potentiometer configured");
  }
  
  this->reset();
  this->set_frequency(1000.0);
  this->set_waveform(WAVEFORM_SINE);
  
  if (this->digipot_ != nullptr) {
    this->set_amplitude(50.0);  // Start at 50%
  }
}

void AD9833::loop() {
  if (sweep_active_) {
    update_sweep_();
  }
  
  if (mod_type_ != MOD_NONE) {
    update_modulation_();
  }
}

void AD9833::dump_config() {
  ESP_LOGCONFIG(TAG, "AD9833:");
  LOG_PIN("  CS Pin: ", this->cs_);
  ESP_LOGCONFIG(TAG, "  Current Frequency: %.2f Hz", this->current_frequency_);
  ESP_LOGCONFIG(TAG, "  Sweep Active: %s", this->sweep_active_ ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  Modulation: %d", this->mod_type_);
  
  if (this->digipot_ != nullptr) {
    LOG_PIN("  Digipot CS Pin: ", this->digipot_cs_pin_);
    ESP_LOGCONFIG(TAG, "  Current Amplitude: %.1f%%", this->get_amplitude());
  }
}

void AD9833::write_register_(uint16_t data) {
  this->enable();
  this->write_byte16(data);
  this->disable();
}

void AD9833::reset() {
  this->write_register_(REG_CONTROL | CTRL_RESET);
  delay(1);
  this->write_register_(REG_CONTROL);
  ESP_LOGD(TAG, "Reset complete");
}

void AD9833::write_frequency_register_(uint32_t freq_word, bool use_freq1) {
  uint16_t lsb = freq_word & 0x3FFF;
  uint16_t msb = (freq_word >> 14) & 0x3FFF;
  uint16_t freq_reg = use_freq1 ? REG_FREQ1 : REG_FREQ0;
  
  // Write control register with B28 bit set for 28-bit write
  uint16_t control = REG_CONTROL | CTRL_B28;
  if (use_freq1) {
    control |= CTRL_FSELECT;
  }
  this->write_register_(control);
  
  // Write frequency LSB
  this->write_register_(freq_reg | lsb);
  
  // Write frequency MSB
  this->write_register_(freq_reg | msb);
}

void AD9833::set_frequency(float frequency) {
  if (frequency < 0.0 || frequency > 12500000.0) {
    ESP_LOGW(TAG, "Frequency out of range (0-12.5MHz): %.2f Hz", frequency);
    return;
  }
  
  uint32_t freq_word = (uint32_t)((frequency * FREQ_RESOLUTION) / FMCLK);
  this->write_frequency_register_(freq_word, false);
  this->current_frequency_ = frequency;
  
  ESP_LOGV(TAG, "Set frequency to %.2f Hz (word: 0x%08X)", frequency, freq_word);
}

void AD9833::write_phase_register_(uint16_t phase_word) {
  this->write_register_(REG_PHASE0 | (phase_word & 0x0FFF));
}

void AD9833::set_phase(float phase_deg) {
  uint16_t phase_word = (uint16_t)((phase_deg / 360.0) * 4096.0);
  this->write_phase_register_(phase_word);
  ESP_LOGD(TAG, "Set phase to %.2f degrees", phase_deg);
}

void AD9833::set_waveform(WaveformType waveform) {
  uint16_t control = REG_CONTROL | CTRL_B28;
  
  switch (waveform) {
    case WAVEFORM_SINE:
      // MODE=0, OPBITEN=0
      break;
    case WAVEFORM_TRIANGLE:
      // MODE=1
      control |= CTRL_MODE;
      break;
    case WAVEFORM_SQUARE:
      // OPBITEN=1, DIV2=1 for MSB/2
      control |= CTRL_OPBITEN | CTRL_DIV2;
      break;
  }
  
  this->write_register_(control);
  this->current_waveform_ = waveform;
  
  const char *waveform_str[] = {"SINE", "TRIANGLE", "SQUARE"};
  ESP_LOGD(TAG, "Set waveform to %s", waveform_str[waveform]);
}

void AD9833::sleep(bool enable) {
  uint16_t control = REG_CONTROL | CTRL_B28;
  
  if (enable) {
    control |= CTRL_SLEEP12; // Power down DAC and internal clock
  }
  
  this->write_register_(control);
  this->is_sleeping_ = enable;
  ESP_LOGD(TAG, "Sleep mode: %s", enable ? "enabled" : "disabled");
}

// Amplitude control
void AD9833::set_amplitude(float percent) {
  if (this->digipot_ == nullptr) {
    ESP_LOGW(TAG, "Digital potentiometer not configured");
    return;
  }
  
  this->digipot_->set_amplitude(percent);
  ESP_LOGD(TAG, "Amplitude set to %.1f%%", percent);
}

void AD9833::set_amplitude_raw(uint8_t value) {
  if (this->digipot_ == nullptr) {
    ESP_LOGW(TAG, "Digital potentiometer not configured");
    return;
  }
  
  this->digipot_->set_wiper(value);
  ESP_LOGD(TAG, "Amplitude wiper set to %d", value);
}

float AD9833::get_amplitude() const {
  if (this->digipot_ == nullptr) {
    return 0.0f;
  }
  
  return (this->digipot_->get_wiper() / 255.0f) * 100.0f;
}

// Sweep implementation
void AD9833::start_sweep(float start_freq, float end_freq, float duration_ms, SweepType type) {
  if (start_freq < 0 || end_freq < 0 || start_freq > 12500000 || end_freq > 12500000) {
    ESP_LOGW(TAG, "Invalid sweep frequency range");
    return;
  }
  
  sweep_start_freq_ = start_freq;
  sweep_end_freq_ = end_freq;
  sweep_duration_ = duration_ms;
  sweep_type_ = type;
  sweep_start_time_ = millis();
  sweep_active_ = true;
  
  ESP_LOGI(TAG, "Starting %s sweep: %.2f Hz -> %.2f Hz over %.0f ms",
           type == SWEEP_LINEAR ? "LINEAR" : "LOGARITHMIC",
           start_freq, end_freq, duration_ms);
}

void AD9833::stop_sweep() {
  sweep_active_ = false;
  ESP_LOGI(TAG, "Sweep stopped");
}

float AD9833::calculate_sweep_frequency_() {
  uint32_t elapsed = millis() - sweep_start_time_;
  float progress = (float)elapsed / sweep_duration_;
  
  // Loop the sweep
  if (progress >= 1.0) {
    sweep_start_time_ = millis();
    progress = 0.0;
  }
  
  float frequency;
  if (sweep_type_ == SWEEP_LINEAR) {
    // Linear interpolation
    frequency = sweep_start_freq_ + (sweep_end_freq_ - sweep_start_freq_) * progress;
  } else {
    // Logarithmic interpolation
    float log_start = log10f(sweep_start_freq_);
    float log_end = log10f(sweep_end_freq_);
    float log_freq = log_start + (log_end - log_start) * progress;
    frequency = powf(10.0f, log_freq);
  }
  
  return frequency;
}

void AD9833::update_sweep_() {
  float new_freq = calculate_sweep_frequency_();
  this->set_frequency(new_freq);
}

// Modulation implementation
void AD9833::set_modulation(ModulationType type, float mod_freq, float depth) {
  if (depth < 0.0 || depth > 1.0) {
    ESP_LOGW(TAG, "Modulation depth must be 0.0-1.0");
    depth = clamp(depth, 0.0f, 1.0f);
  }
  
  mod_type_ = type;
  mod_frequency_ = mod_freq;
  mod_depth_ = depth;
  mod_last_update_ = millis();
  mod_phase_ = 0.0;
  
  const char *mod_str[] = {"NONE", "AM", "FM", "FSK"};
  ESP_LOGI(TAG, "Modulation set: %s, Freq: %.2f Hz, Depth: %.2f", 
           mod_str[type], mod_freq, depth);
  
  if (type == MOD_AM && this->digipot_ != nullptr) {
    ESP_LOGI(TAG, "Using MCP41010 for true amplitude modulation");
  }
}

void AD9833::stop_modulation() {
  mod_type_ = MOD_NONE;
  ESP_LOGI(TAG, "Modulation stopped");
}

void AD9833::update_modulation_() {
  uint32_t now = millis();
  float dt = (now - mod_last_update_) / 1000.0f; // seconds
  mod_last_update_ = now;
  
  // Update modulation phase
  mod_phase_ += 2.0f * PI * mod_frequency_ * dt;
  if (mod_phase_ > 2.0f * PI) {
    mod_phase_ -= 2.0f * PI;
  }
  
  float mod_signal = sinf(mod_phase_);
  
  switch (mod_type_) {
    case MOD_AM:
      if (this->digipot_ != nullptr) {
        // True amplitude modulation using digital potentiometer
        // Map mod_signal (-1 to 1) to amplitude range
        float base_amplitude = 50.0f;  // Center amplitude
        float amplitude = base_amplitude + (base_amplitude * mod_depth_ * mod_signal);
        this->set_amplitude(amplitude);
      } else {
        // Fallback to phase modulation if no digipot
        float phase_mod = mod_signal * mod_depth_ * 180.0f;
        this->set_phase(phase_mod);
      }
      break;
      
    case MOD_FM:
      // FM: Vary frequency around carrier
      {
        float freq_deviation = current_frequency_ * mod_depth_;
        float new_freq = current_frequency_ + (freq_deviation * mod_signal);
        this->set_frequency(new_freq);
      }
      break;
      
    case MOD_FSK:
      // FSK is handled separately via set_fsk_state()
      break;
      
    default:
      break;
  }
}

// FSK implementation
void AD9833::set_fsk_frequencies(float freq1, float freq2) {
  fsk_freq1_ = freq1;
  fsk_freq2_ = freq2;
  
  // Pre-load both frequency registers
  uint32_t freq_word1 = (uint32_t)((freq1 * FREQ_RESOLUTION) / FMCLK);
  uint32_t freq_word2 = (uint32_t)((freq2 * FREQ_RESOLUTION) / FMCLK);
  
  this->write_frequency_register_(freq_word1, false);
  this->write_frequency_register_(freq_word2, true);
  
  ESP_LOGI(TAG, "FSK frequencies set: %.2f Hz / %.2f Hz", freq1, freq2);
}

void AD9833::set_fsk_state(bool state) {
  fsk_state_ = state;
  
  // Switch between FREQ0 and FREQ1 registers using FSELECT bit
  uint16_t control = REG_CONTROL | CTRL_B28;
  if (state) {
    control |= CTRL_FSELECT;
  }
  this->write_register_(control);
  
  ESP_LOGV(TAG, "FSK state: %d", state);
}

}  // namespace ad9833
}  // namespace esphome

