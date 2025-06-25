#pragma once

#include <cmath>  // For sine function
#include <vector>

enum class WaveType {
  SINE = 0,
  COSINE,
  SQUARE,
  TRIANGLE,
  SAWTOOTH
};

class CoreLogic {
 public:
  CoreLogic();

  // Method to update sine wave values based on frequency and amplitude
  void Update();

  float& GetFrequency() { return frequency_; };
  float& GetAmplitude() { return amplitude_; };
  float& GetFps() { return fps_; };
  float& GetPhase() { return phase_; };
  float& GetNoise() { return noise_; };
  WaveType& GetWaveType() { return wave_type_; };
  
  // Color getters/setters
  float* GetWaveColor() { return wave_color_; };
  float* GetBgColor() { return bg_color_; };
  void SetWaveColor(float r, float g, float b) { 
    wave_color_[0] = r; wave_color_[1] = g; wave_color_[2] = b; 
  };
  void SetBgColor(float r, float g, float b) { 
    bg_color_[0] = r; bg_color_[1] = g; bg_color_[2] = b; 
  };

  // Getter for sine wave values
  inline const std::vector<float> GetSineWaveValues() const {
    return sine_wave_values_;
  };

 private:
  static constexpr size_t MAX_VALUES = 500;
  
  // Wave generation function
  float GenerateWaveValue(float time) const;
  
  // Simulation parameters
  float frequency_ = 1.f;
  float amplitude_ = 1.f;
  float phase_ = 0.f;
  float noise_ = 0.f;
  float time_;
  float fps_ = 60.f;
  WaveType wave_type_ = WaveType::SINE;
  
  // Color parameters
  float wave_color_[3] = {0.26f, 0.59f, 0.98f}; // Default blue
  float bg_color_[3] = {0.12f, 0.14f, 0.18f};   // Default dark gray
  
  std::vector<float> sine_wave_values_;
};