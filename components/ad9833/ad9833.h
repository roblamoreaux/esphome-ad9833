# ad9833.h
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"
#include <vector>

namespace esphome {
namespace ad9833 {

enum WaveformType {
  WAVEFORM_SINE = 0,
  WAVEFORM_TRIANGLE = 1,
  WAVEFORM_SQUARE = 2
};

enum SweepType {
  SWEEP_LINEAR = 0,
  SWEEP_LOGARITHMIC = 1
};

enum ModulationType {
  MOD_NONE = 0,
  MOD_AM = 1,      // Amplitude Modulation
  MOD_FM = 2,      // Frequency Modulation
  MOD_FSK = 3      // Frequency Shift Keying
};

class MCP41010 {
 public:
  MCP41010(spi::SPIComponent *parent, GPIOPin *cs_pin);
  void setup();
  void set_wiper(uint8_t value);  // 0-255
  void set_amplitude(float percent);  // 0.0-100.0
  uint8_t get_wiper() const { return current_wiper_; }
  
 protected:
  spi::SPIComponent *parent_;
  GPIOPin *cs_pin_;
  uint8_t current_wiper_{128};
  
  static const uint8_t CMD_WRITE = 0x11;
  static const uint8_t CMD_SHUTDOWN = 0x21;
};

class AD9833 : public Component,
               public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH,
                                     spi::CLOCK_PHASE_SECOND_EDGE, spi::DATA_RATE_1MHZ> {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  // Basic control
  void set_frequency(float frequency);
  void set_waveform(WaveformType waveform);
  void set_phase(float phase_deg);
  void sleep(bool enable);
  void reset();

  // Amplitude control via MCP41010
  void set_amplitude(float percent);  // 0.0-100.0
  void set_amplitude_raw(uint8_t value);  // 0-255
  float get_amplitude() const;
  
  // Digital potentiometer setup
  void set_digipot_cs_pin(GPIOPin *cs_pin) { digipot_cs_pin_ = cs_pin; }

  // Sweep control
  void start_sweep(float start_freq, float end_freq, float duration_ms, SweepType type = SWEEP_LINEAR);
  void stop_sweep();
  bool is_sweeping() const { return sweep_active_; }

  // Modulation control
  void set_modulation(ModulationType type, float mod_freq, float depth);
  void stop_modulation();
  bool is_modulating() const { return mod_type_ != MOD_NONE; }

  // FSK specific
  void set_fsk_frequencies(float freq1, float freq2);
  void set_fsk_state(bool state);

 protected:
  void write_register_(uint16_t data);
  void write_frequency_register_(uint32_t freq_word, bool use_freq1 = false);
  void write_phase_register_(uint16_t phase_word);
  void update_sweep_();
  void update_modulation_();
  float calculate_sweep_frequency_();
  
  const float FMCLK = 25000000.0; // 25 MHz crystal
  const uint32_t FREQ_RESOLUTION = 268435456; // 2^28
  
  // Current state
  WaveformType current_waveform_{WAVEFORM_SINE};
  float current_frequency_{1000.0};
  bool is_sleeping_{false};

  // Digital potentiometer
  GPIOPin *digipot_cs_pin_{nullptr};
  MCP41010 *digipot_{nullptr};

  // Sweep parameters
  bool sweep_active_{false};
  SweepType sweep_type_{SWEEP_LINEAR};
  float sweep_start_freq_{1000.0};
  float sweep_end_freq_{10000.0};
  float sweep_duration_{1000.0};
  uint32_t sweep_start_time_{0};

  // Modulation parameters
  ModulationType mod_type_{MOD_NONE};
  float mod_frequency_{10.0};
  float mod_depth_{0.5};
  uint32_t mod_last_update_{0};
  float mod_phase_{0.0};
  bool mod_use_amplitude_{false};  // Use amplitude for AM instead of phase
  
  // FSK parameters
  float fsk_freq1_{1000.0};
  float fsk_freq2_{2000.0};
  bool fsk_state_{false};
};

}  // namespace ad9833
}  // namespace esphome

