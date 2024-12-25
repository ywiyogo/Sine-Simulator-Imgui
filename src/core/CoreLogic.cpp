#include "CoreLogic.hpp"

#include <fmt/base.h>
#include <fmt/core.h>

CoreLogic::CoreLogic() {
  sine_wave_values_.clear();
  time_ = 0.f;
}

void CoreLogic::Update() {
  // Assuming ~60 FPS or 1/60 of a sec
  time_ += 1 / fps_;
  float value = amplitude_ * std::sin(2.0f * M_PI * frequency_ * time_);
  sine_wave_values_.push_back(value);
  if (sine_wave_values_.size() > MAX_VALUES) {
    sine_wave_values_.erase(sine_wave_values_.begin());
  }
}