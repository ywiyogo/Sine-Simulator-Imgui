#pragma once
#include "imgui.h"
#include <chrono>
#include <cmath>
#include <algorithm>

namespace ImGui {

// Animation System
class Animations {
public:
    static Animations& Instance() {
        static Animations instance;
        return instance;
    }

    float GetTime() {
        auto now = std::chrono::steady_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration<float>(duration).count();
    }

    float EaseInOutCubic(float t) {
        return t < 0.5f ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3) / 2;
    }

    float EaseInOutSine(float t) {
        return -(std::cos(3.14159f * t) - 1) / 2;
    }

    ImVec4 LerpColor(const ImVec4& a, const ImVec4& b, float t) {
        return ImVec4(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t,
            a.w + (b.w - a.w) * t
        );
    }

    float PulseAnimation(float speed = 2.0f, float min_val = 0.7f, float max_val = 1.0f) {
        float time = GetTime() * speed;
        float pulse = (std::sin(time) + 1.0f) * 0.5f;
        return min_val + (max_val - min_val) * pulse;
    }

    ImVec4 GetGradientColor(const ImVec4& color1, const ImVec4& color2, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return LerpColor(color1, color2, EaseInOutSine(t));
    }

    // Additional easing functions
    float EaseInQuad(float t) {
        return t * t;
    }

    float EaseOutQuad(float t) {
        return 1 - (1 - t) * (1 - t);
    }

    float EaseInOutQuad(float t) {
        return t < 0.5f ? 2 * t * t : 1 - std::pow(-2 * t + 2, 2) / 2;
    }

    float EaseInBack(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1;
        return c3 * t * t * t - c1 * t * t;
    }

    float EaseOutBack(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1;
        return 1 + c3 * std::pow(t - 1, 3) + c1 * std::pow(t - 1, 2);
    }

    float EaseInOutBack(float t) {
        const float c1 = 1.70158f;
        const float c2 = c1 * 1.525f;
        return t < 0.5f
            ? (std::pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2
            : (std::pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2;
    }

    // Bounce animation
    float EaseOutBounce(float t) {
        const float n1 = 7.5625f;
        const float d1 = 2.75f;

        if (t < 1 / d1) {
            return n1 * t * t;
        } else if (t < 2 / d1) {
            t -= 1.5f / d1;
            return n1 * t * t + 0.75f;
        } else if (t < 2.5 / d1) {
            t -= 2.25f / d1;
            return n1 * t * t + 0.9375f;
        } else {
            t -= 2.625f / d1;
            return n1 * t * t + 0.984375f;
        }
    }

    float EaseInBounce(float t) {
        return 1 - EaseOutBounce(1 - t);
    }

    float EaseInOutBounce(float t) {
        return t < 0.5f
            ? (1 - EaseOutBounce(1 - 2 * t)) / 2
            : (1 + EaseOutBounce(2 * t - 1)) / 2;
    }

    // Utility functions for common animations
    float FadeInOut(float duration, float delay = 0.0f) {
        float time = GetTime() - delay;
        if (time < 0) return 0.0f;
        if (time > duration) return 0.0f;
        
        float normalized = time / duration;
        return normalized < 0.5f ? EaseInQuad(normalized * 2) : EaseOutQuad((1 - normalized) * 2);
    }

    float SlideIn(float duration, float delay = 0.0f) {
        float time = GetTime() - delay;
        if (time < 0) return 0.0f;
        if (time > duration) return 1.0f;
        
        return EaseOutBack(time / duration);
    }

    float SpringAnimation(float target, float current, float stiffness = 0.1f, float damping = 0.8f) {
        float delta = target - current;
        return current + delta * stiffness * damping;
    }

private:
    Animations() = default;
    ~Animations() = default;
    Animations(const Animations&) = delete;
    Animations& operator=(const Animations&) = delete;
};

} // namespace ImGui