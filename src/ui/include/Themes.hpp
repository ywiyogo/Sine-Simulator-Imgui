#pragma once
#include "imgui.h"
#include <cstdint>

namespace ImGui {

// Hex to ImVec4 conversion utility
constexpr ImVec4 HexToImVec4(uint32_t hex, float alpha = 1.0f) {
    return ImVec4(
        ((hex >> 16) & 0xFF) / 255.0f,  // Red
        ((hex >> 8) & 0xFF) / 255.0f,   // Green
        (hex & 0xFF) / 255.0f,          // Blue
        alpha                           // Alpha
    );
}

// Color manipulation utilities
namespace ColorUtils {
    // Lighten a color by a percentage (0.0 to 1.0)
    constexpr ImVec4 Lighten(const ImVec4& color, float amount) {
        return ImVec4(
            color.x + (1.0f - color.x) * amount,
            color.y + (1.0f - color.y) * amount,
            color.z + (1.0f - color.z) * amount,
            color.w
        );
    }
    
    // Darken a color by a percentage (0.0 to 1.0)
    constexpr ImVec4 Darken(const ImVec4& color, float amount) {
        return ImVec4(
            color.x * (1.0f - amount),
            color.y * (1.0f - amount),
            color.z * (1.0f - amount),
            color.w
        );
    }
    
    // Increase saturation (make more vibrant)
    constexpr ImVec4 Saturate(const ImVec4& color, float amount) {
        float gray = 0.299f * color.x + 0.587f * color.y + 0.114f * color.z;
        return ImVec4(
            color.x + (color.x - gray) * amount,
            color.y + (color.y - gray) * amount,
            color.z + (color.z - gray) * amount,
            color.w
        );
    }
}

// Color Theme Definitions
struct ColorTheme {
    ImVec4 primary;
    ImVec4 secondary;
    ImVec4 hover;
    const char* name;
    
    constexpr ColorTheme(uint32_t primaryHex, const char* themeName) 
        : primary(HexToImVec4(primaryHex))
        , secondary(ColorUtils::Lighten(HexToImVec4(primaryHex), 0.15f))
        , hover(ColorUtils::Lighten(HexToImVec4(primaryHex), 0.08f))
        , name(themeName) {}
};

// Predefined color themes
namespace ColorThemes {
    constexpr ColorTheme BLUE = ColorTheme(0x4287F5, "Ocean Blue");      // #4287F5
    constexpr ColorTheme ORANGE = ColorTheme(0xD2691E, "Sunset Orange"); // #D2691E  
    constexpr ColorTheme LIME = ColorTheme(0x6B8E23, "Electric Lime");   // #6B8E23
    constexpr ColorTheme PURPLE = ColorTheme(0x8B5CF6, "Royal Purple");  // #8B5CF6
    constexpr ColorTheme ROSE = ColorTheme(0xF43F5E, "Romantic Rose");   // #F43F5E
    constexpr ColorTheme EMERALD = ColorTheme(0x10B981, "Forest Emerald"); // #10B981
    constexpr ColorTheme AMBER = ColorTheme(0xF59E0B, "Golden Amber");   // #F59E0B
    constexpr ColorTheme INDIGO = ColorTheme(0x6366F1, "Deep Indigo");   // #6366F1
}

// Theme System
class Themes {
private:
    static inline ColorTheme currentTheme = ColorThemes::BLUE;
    
public:
    static void SetTheme(const ColorTheme& theme) {
        currentTheme = theme;
    }
    
    static const ColorTheme& GetCurrentTheme() {
        return currentTheme;
    }
    
    static constexpr ColorTheme themes[] = {
        ColorThemes::BLUE,
        ColorThemes::ORANGE,
        ColorThemes::LIME,
        ColorThemes::ROSE,
        ColorThemes::EMERALD,
        ColorThemes::INDIGO
    };
    
    static constexpr size_t GetThemeCount() {
        return sizeof(themes) / sizeof(themes[0]);
    }
};

// Modern color palette
namespace Colors {
    // Primary colors with gradient support
    constexpr ImVec4 PRIMARY_DARK = ImVec4(0.12f, 0.14f, 0.18f, 1.0f);
    constexpr ImVec4 PRIMARY_MEDIUM = ImVec4(0.18f, 0.20f, 0.25f, 1.0f);
    constexpr ImVec4 PRIMARY_LIGHT = ImVec4(0.25f, 0.28f, 0.32f, 1.0f);
    
    // Dynamic accent colors (will be updated by theme)
    inline ImVec4 ACCENT_PRIMARY = Themes::GetCurrentTheme().primary;
    inline ImVec4 ACCENT_SECONDARY = Themes::GetCurrentTheme().secondary;
    inline ImVec4 ACCENT_HOVER = Themes::GetCurrentTheme().hover;
    
    // Static method to update accent colors
    static void UpdateAccentColors() {
        ACCENT_PRIMARY = Themes::GetCurrentTheme().primary;
        ACCENT_SECONDARY = Themes::GetCurrentTheme().secondary;
        ACCENT_HOVER = Themes::GetCurrentTheme().hover;
    }

    // Success/Error colors
    constexpr ImVec4 SUCCESS = ImVec4(0.20f, 0.80f, 0.20f, 1.0f);
    constexpr ImVec4 WARNING = ImVec4(1.0f, 0.65f, 0.0f, 1.0f);
    constexpr ImVec4 ERROR = ImVec4(0.95f, 0.30f, 0.30f, 1.0f);

    // Text colors
    constexpr ImVec4 TEXT_PRIMARY = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    constexpr ImVec4 TEXT_SECONDARY = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
    constexpr ImVec4 TEXT_DISABLED = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);

    // Sidebar colors
    constexpr ImVec4 SIDEBAR_BG = ImVec4(0.08f, 0.10f, 0.14f, 0.95f);
    constexpr ImVec4 SIDEBAR_HEADER = ImVec4(0.15f, 0.17f, 0.22f, 1.0f);

    // Glass effect colors
    constexpr ImVec4 GLASS_BG = ImVec4(0.20f, 0.22f, 0.27f, 0.80f);
    constexpr ImVec4 GLASS_BORDER = ImVec4(0.40f, 0.42f, 0.47f, 0.30f);
}

} // namespace ImGui