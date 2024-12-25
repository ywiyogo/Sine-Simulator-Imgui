#pragma once

#include <cmath>  // For sine function
#include <vector>

class CoreLogic {
 public:
  CoreLogic();

  // Method to update sine wave values based on frequency and amplitude
  void Update();

  float& GetFrequency() { return frequency_; };
  float& GetAmplitude() { return amplitude_; };
  float& GetFps() { return fps_; };

  // Getter for sine wave values
  inline const std::vector<float> GetSineWaveValues() const {
    return sine_wave_values_;
  };

 private:
  static constexpr size_t MAX_VALUES = 500;
  // Simulation parameters
  float frequency_ = 1.f;
  float amplitude_ = 1.f;
  float time_;
  float fps_ = 60.f;
  std::vector<float> sine_wave_values_;
};