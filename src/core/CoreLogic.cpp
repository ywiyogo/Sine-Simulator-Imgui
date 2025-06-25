#include "CoreLogic.hpp"

#include <fmt/core.h>
#include <random>

CoreLogic::CoreLogic() {
  sine_wave_values_.clear();
  time_ = 0.f;
}

void CoreLogic::Update() {
  // Assuming ~60 FPS or 1/60 of a sec
  time_ += 1 / fps_;
  float value = GenerateWaveValue(time_);
  sine_wave_values_.push_back(value);
  if (sine_wave_values_.size() > MAX_VALUES) {
    sine_wave_values_.erase(sine_wave_values_.begin());
  }
}

float CoreLogic::GenerateWaveValue(float time) const {
  float base_value = 0.0f;
  float adjusted_time = 2.0f * M_PI * frequency_ * time + phase_;
  
  switch (wave_type_) {
    case WaveType::SINE:
      base_value = std::sin(adjusted_time);
      break;
    case WaveType::COSINE:
      base_value = std::cos(adjusted_time);
      break;
    case WaveType::SQUARE:
      base_value = std::sin(adjusted_time) >= 0.0f ? 1.0f : -1.0f;
      break;
    case WaveType::TRIANGLE:
      {
        float normalized = fmod(adjusted_time / (2.0f * M_PI), 1.0f);
        if (normalized < 0) normalized += 1.0f;
        if (normalized < 0.25f) {
          base_value = 4.0f * normalized;
        } else if (normalized < 0.75f) {
          base_value = 2.0f - 4.0f * normalized;
        } else {
          base_value = 4.0f * normalized - 4.0f;
        }
      }
      break;
    case WaveType::SAWTOOTH:
      {
        float normalized = fmod(adjusted_time / (2.0f * M_PI), 1.0f);
        if (normalized < 0) normalized += 1.0f;
        base_value = 2.0f * normalized - 1.0f;
      }
      break;
  }
  
  // Apply amplitude
  base_value *= amplitude_;
  
  // Add noise if enabled
  if (noise_ > 0.0f) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    base_value += noise_ * amplitude_ * dis(gen);
  }
  
  return base_value;
}