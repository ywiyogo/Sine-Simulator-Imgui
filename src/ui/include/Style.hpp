#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "Themes.hpp"
#include "Animations.hpp"
#include <cmath>

// Helper functions for ImVec2 operations
inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) {
    return ImVec2(a.x + b.x, a.y + b.y);
}

inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) {
    return ImVec2(a.x - b.x, a.y - b.y);
}

inline ImVec2 operator*(const ImVec2& a, float b) {
    return ImVec2(a.x * b, a.y * b);
}

namespace ImGui {

// Custom button with gradient and animation
inline bool GradientButton(const char* label, const ImVec2& size = ImVec2(0, 0)) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, nullptr, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size_arg = CalcItemSize(size, label_size.x + style.FramePadding.x * 2.0f,
                                   label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size_arg);
    ItemSize(size_arg, style.FramePadding.y);
    if (!ItemAdd(bb, id)) return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    // Gradient colors based on state
    ImVec4 color_top, color_bottom;
    if (held) {
        color_top = Colors::ACCENT_PRIMARY;
        color_bottom = ImVec4(Colors::ACCENT_PRIMARY.x * 0.8f, Colors::ACCENT_PRIMARY.y * 0.8f,
                             Colors::ACCENT_PRIMARY.z * 0.8f, Colors::ACCENT_PRIMARY.w);
    } else if (hovered) {
        float pulse = Animations::Instance().PulseAnimation(3.0f, 0.9f, 1.1f);
        color_top = ImVec4(Colors::ACCENT_HOVER.x * pulse, Colors::ACCENT_HOVER.y * pulse,
                          Colors::ACCENT_HOVER.z * pulse, Colors::ACCENT_HOVER.w);
        color_bottom = Colors::ACCENT_PRIMARY;
    } else {
        color_top = Colors::ACCENT_SECONDARY;
        color_bottom = Colors::ACCENT_PRIMARY;
    }

    // Draw gradient background
    ImDrawList* draw_list = window->DrawList;
    draw_list->AddRectFilledMultiColor(
        bb.Min, bb.Max,
        GetColorU32(color_top), GetColorU32(color_top),
        GetColorU32(color_bottom), GetColorU32(color_bottom)
    );

    // Add subtle border with glow effect
    if (hovered) {
        draw_list->AddRect(bb.Min, bb.Max, GetColorU32(Colors::ACCENT_HOVER), 4.0f, 0, 2.0f);
    } else {
        draw_list->AddRect(bb.Min, bb.Max, GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 0.3f)), 4.0f, 0, 1.0f);
    }

    // Render text
    PushStyleColor(ImGuiCol_Text, Colors::TEXT_PRIMARY);
    RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding,
                     label, nullptr, &label_size, style.ButtonTextAlign, &bb);
    PopStyleColor();

    return pressed;
}

// Glass effect panel
inline void BeginGlassPanel(const char* name, const ImVec2& pos, const ImVec2& size) {
    ImGuiWindow* window = GetCurrentWindow();
    ImDrawList* draw_list = window->DrawList;

    // Glass background with blur effect simulation
    draw_list->AddRectFilled(pos, pos + size, GetColorU32(Colors::GLASS_BG), 8.0f);

    // Border with subtle glow
    draw_list->AddRect(pos, pos + size, GetColorU32(Colors::GLASS_BORDER), 8.0f, 0, 1.5f);

    SetCursorScreenPos(pos + ImVec2(10, 10));
}

// Animated progress bar
inline void AnimatedProgressBar(float fraction, const ImVec2& size, const char* overlay = nullptr) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size_arg = CalcItemSize(size, CalcItemWidth(), g.FontSize + style.FramePadding.y * 2.0f);
    ImRect bb(pos, pos + size_arg);
    ItemSize(size_arg, style.FramePadding.y);
    if (!ItemAdd(bb, 0)) return;

    // Background
    ImDrawList* draw_list = window->DrawList;
    draw_list->AddRectFilled(bb.Min, bb.Max, GetColorU32(Colors::PRIMARY_DARK), 4.0f);

    // Animated progress with gradient
    if (fraction > 0.0f) {
        ImVec2 fill_end = ImVec2(bb.Min.x + (bb.Max.x - bb.Min.x) * fraction, bb.Max.y);

        // Gradient progress bar
        ImVec4 color1 = Colors::ACCENT_PRIMARY;
        ImVec4 color2 = Colors::ACCENT_SECONDARY;

        draw_list->AddRectFilledMultiColor(
            bb.Min, fill_end,
            GetColorU32(color1), GetColorU32(color2),
            GetColorU32(color2), GetColorU32(color1)
        );

        // Animated shine effect
        float shine_pos = fmod(Animations::Instance().GetTime() * 2.0f, 2.0f) - 1.0f;
        if (shine_pos > 0.0f && shine_pos < 1.0f) {
            float shine_x = bb.Min.x + (bb.Max.x - bb.Min.x) * shine_pos * fraction;
            ImVec2 shine_start(shine_x - 20, bb.Min.y);
            ImVec2 shine_end(shine_x + 20, bb.Max.y);

            draw_list->AddRectFilledMultiColor(
                shine_start, shine_end,
                GetColorU32(ImVec4(1, 1, 1, 0)), GetColorU32(ImVec4(1, 1, 1, 0.3f)),
                GetColorU32(ImVec4(1, 1, 1, 0.3f)), GetColorU32(ImVec4(1, 1, 1, 0))
            );
        }
    }

    // Border
    draw_list->AddRect(bb.Min, bb.Max, GetColorU32(Colors::GLASS_BORDER), 4.0f);

    // Overlay text
    if (overlay) {
        ImVec2 overlay_size = CalcTextSize(overlay);
        ImVec2 overlay_pos = bb.Min + (size_arg - overlay_size) * 0.5f;
        draw_list->AddText(overlay_pos, GetColorU32(Colors::TEXT_PRIMARY), overlay);
    }
}

// Setup modern ImGui style
inline void SetupImGuiStyle(bool is_dark_style = true, float alpha_threshold = 1.0f) {
    ImGuiStyle& style = GetStyle();
    ImVec4* colors = style.Colors;

    // Modern styling parameters
    style.WindowRounding = 12.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;

    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    style.WindowPadding = ImVec2(15, 15);
    style.FramePadding = ImVec2(12, 8);
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 16.0f;
    style.GrabMinSize = 12.0f;

    // Modern color scheme
    colors[ImGuiCol_Text] = Colors::TEXT_PRIMARY;
    colors[ImGuiCol_TextDisabled] = Colors::TEXT_DISABLED;
    colors[ImGuiCol_WindowBg] = Colors::PRIMARY_DARK;
    colors[ImGuiCol_ChildBg] = Colors::PRIMARY_MEDIUM;
    colors[ImGuiCol_PopupBg] = Colors::PRIMARY_LIGHT;
    colors[ImGuiCol_Border] = Colors::GLASS_BORDER;
    colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_FrameBg] = Colors::PRIMARY_MEDIUM;
    colors[ImGuiCol_FrameBgHovered] = Colors::PRIMARY_LIGHT;
    colors[ImGuiCol_FrameBgActive] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_TitleBg] = Colors::PRIMARY_DARK;
    colors[ImGuiCol_TitleBgActive] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_TitleBgCollapsed] = Colors::PRIMARY_MEDIUM;
    colors[ImGuiCol_MenuBarBg] = Colors::SIDEBAR_HEADER;
    colors[ImGuiCol_ScrollbarBg] = Colors::PRIMARY_DARK;
    colors[ImGuiCol_ScrollbarGrab] = Colors::PRIMARY_LIGHT;
    colors[ImGuiCol_ScrollbarGrabHovered] = Colors::ACCENT_HOVER;
    colors[ImGuiCol_ScrollbarGrabActive] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_CheckMark] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_SliderGrab] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_SliderGrabActive] = Colors::ACCENT_SECONDARY;
    colors[ImGuiCol_Button] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_ButtonHovered] = Colors::ACCENT_HOVER;
    colors[ImGuiCol_ButtonActive] = Colors::ACCENT_SECONDARY;
    colors[ImGuiCol_Header] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_HeaderHovered] = Colors::ACCENT_HOVER;
    colors[ImGuiCol_HeaderActive] = Colors::ACCENT_SECONDARY;
    colors[ImGuiCol_Separator] = Colors::GLASS_BORDER;
    colors[ImGuiCol_SeparatorHovered] = Colors::ACCENT_HOVER;
    colors[ImGuiCol_SeparatorActive] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_ResizeGrip] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_ResizeGripHovered] = Colors::ACCENT_HOVER;
    colors[ImGuiCol_ResizeGripActive] = Colors::ACCENT_SECONDARY;
    colors[ImGuiCol_Tab] = Colors::PRIMARY_MEDIUM;
    colors[ImGuiCol_TabHovered] = Colors::ACCENT_HOVER;
    colors[ImGuiCol_TabActive] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_TabUnfocused] = Colors::PRIMARY_DARK;
    colors[ImGuiCol_TabUnfocusedActive] = Colors::PRIMARY_LIGHT;
    colors[ImGuiCol_PlotLines] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_PlotLinesHovered] = Colors::ACCENT_SECONDARY;
    colors[ImGuiCol_PlotHistogram] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_PlotHistogramHovered] = Colors::ACCENT_SECONDARY;
    colors[ImGuiCol_TextSelectedBg] = ImVec4(Colors::ACCENT_PRIMARY.x, Colors::ACCENT_PRIMARY.y, Colors::ACCENT_PRIMARY.z, 0.35f);
    colors[ImGuiCol_DragDropTarget] = Colors::ACCENT_SECONDARY;
    colors[ImGuiCol_NavHighlight] = Colors::ACCENT_PRIMARY;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    // Apply alpha threshold
    for (int i = 0; i < ImGuiCol_COUNT; i++) {
        colors[i].w *= alpha_threshold;
    }
}

// Utility functions for enhanced visuals
inline void DrawGlow(ImDrawList* draw_list, const ImVec2& center, float radius, const ImVec4& color, int segments = 32) {
    for (int i = 0; i < 3; i++) {
        float alpha = (3 - i) * 0.1f;
        float size = radius + i * 3.0f;
        ImVec4 glow_color = ImVec4(color.x, color.y, color.z, color.w * alpha);
        draw_list->AddCircleFilled(center, size, GetColorU32(glow_color), segments);
    }
}

inline void DrawSineWaveBackground(ImDrawList* draw_list, const ImRect& bb, float time, const ImVec4& color) {
    const int point_count = 200;
    const float frequency = 2.0f;
    const float amplitude = 20.0f;

    for (int i = 0; i < point_count - 1; i++) {
        float t1 = (float)i / (point_count - 1);
        float t2 = (float)(i + 1) / (point_count - 1);

        float x1 = bb.Min.x + t1 * (bb.Max.x - bb.Min.x);
        float x2 = bb.Min.x + t2 * (bb.Max.x - bb.Min.x);

        float y1 = bb.Min.y + (bb.Max.y - bb.Min.y) * 0.5f + amplitude * sin(frequency * t1 * 6.28f + time);
        float y2 = bb.Min.y + (bb.Max.y - bb.Min.y) * 0.5f + amplitude * sin(frequency * t2 * 6.28f + time);

        ImVec4 line_color = ImVec4(color.x, color.y, color.z, color.w * 0.1f);
        draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), GetColorU32(line_color), 1.0f);
    }
}

} // namespace ImGui
