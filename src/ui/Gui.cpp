#include "Gui.hpp"

#include <numeric>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <string>

#include "imgui.h"
#include "imgui_internal.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
// avoid using FMT_STRING macros from fmt/format.h and fmt::println
#include <fmt/color.h>
#include <fmt/core.h>

#include "Style.hpp"

Gui::Gui(CoreLogic& coreLogic)
    : window(nullptr),
      renderer(nullptr),
      running(true),
      paused(false),
      core_logic_(coreLogic) {}

Gui::~Gui() { Cleanup(); }

bool Gui::Initialize() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fmt::print("SDL initialization failed: {}", SDL_GetError());
    return false;
  }

  window = SDL_CreateWindow("Wave Simulator Pro", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 1920, 1080,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  if (!window) {
    fmt::print("Window creation failed: {}\n", SDL_GetError());
    return false;
  }

  fmt::print("Creating renderer...\n");
#ifdef __EMSCRIPTEN__
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
#else
  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
#endif

  if (!renderer) {
    fmt::print("Renderer creation failed: {}\n", SDL_GetError());
    return false;
  }

  fmt::print("Initializing ImGui...\n");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  if (!ImGui_ImplSDL2_InitForSDLRenderer(window, renderer)) {
    fmt::print("ImGui SDL2 initialization failed\n");
    return false;
  }

  if (!ImGui_ImplSDLRenderer2_Init(renderer)) {
    fmt::print("ImGui SDL2 Renderer initialization failed\n");
    return false;
  }

  LoadSystemFonts();
  ImGui::SetupImGuiStyle(true);

  // Initialize default theme
  LoadThemePreference();

  fmt::print(fg(fmt::color::green), "Initialization complete!\n");
  return true;
}

void Gui::ProcessEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT) {
      running = false;
    }

    // Handle keyboard shortcuts
    if (event.type == SDL_KEYDOWN) {
      const Uint8* keyState = SDL_GetKeyboardState(NULL);

      // Ctrl+T: Cycle to next theme
      if (event.key.keysym.sym == SDLK_t && (keyState[SDL_SCANCODE_LCTRL] || keyState[SDL_SCANCODE_RCTRL])) {
        int nextTheme = (currentThemeIndex + 1) % ImGui::Themes::GetThemeCount();
        ApplyTheme(nextTheme);
        SaveThemePreference();
      }

      // Ctrl+Shift+T: Cycle to previous theme
      if (event.key.keysym.sym == SDLK_t && (keyState[SDL_SCANCODE_LCTRL] || keyState[SDL_SCANCODE_RCTRL]) && (keyState[SDL_SCANCODE_LSHIFT] || keyState[SDL_SCANCODE_RSHIFT])) {
        int prevTheme = (currentThemeIndex - 1 + ImGui::Themes::GetThemeCount()) % ImGui::Themes::GetThemeCount();
        ApplyTheme(prevTheme);
        SaveThemePreference();
      }
    }
  }
}

void Gui::Update() {
  animationTime += ImGui::GetIO().DeltaTime;

  // Smooth sidebar animation
  if (!isInitialAnimationComplete) {
    sidebarAnimationOffset = ImGui::Animations::Instance().EaseInOutCubic(
      std::min(animationTime / 2.0f, 1.0f));
    if (animationTime > 2.0f) {
      isInitialAnimationComplete = true;
    }
  }

  // Handle theme changes
  if (themeChanged) {
    ApplyTheme(currentThemeIndex);
    SaveThemePreference();
  }

  // Update theme notification timer
  if (showThemeNotification) {
    themeNotificationTimer -= ImGui::GetIO().DeltaTime;
    if (themeNotificationTimer <= 0.0f) {
      showThemeNotification = false;
    }
  }
}

void Gui::Render() {
  SDL_SetRenderDrawColor(renderer, 20, 25, 30, 255);
  if (SDL_RenderClear(renderer) != 0) {
    fmt::print("Render clear failed: {}\n", SDL_GetError());
    return;
  }

  // Start ImGui frame
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  try {
    RenderMainInterface();
  } catch (const std::exception& e) {
    fmt::print("ImGui rendering error: {}\n", e.what());
  }

  // Render ImGui
  ImGui::Render();
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

  SDL_RenderPresent(renderer);
}

void Gui::RenderMainInterface() {
  ImGuiIO& io = ImGui::GetIO();
  ImVec2 displaySize = io.DisplaySize;
  float ymargin = 30.0f;

  // Create main menu bar
#ifndef __EMSCRIPTEN__
  RenderMenuBar();
#endif

  // Main layout similar to original but with enhanced styling
  ImGui::SetNextWindowPos(ImVec2(0, ymargin));
  ImGui::SetNextWindowSize(ImVec2(displaySize.x, displaySize.y - ymargin));

  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                                ImGuiWindowFlags_NoResize |
                                ImGuiWindowFlags_NoMove |
                                ImGuiWindowFlags_NoCollapse;

  ImGui::Begin("MainInterface", nullptr, windowFlags);

  float availableWidth = ImGui::GetContentRegionAvail().x;
  float availableHeight = ImGui::GetContentRegionAvail().y;

  // Calculate responsive panel sizes
  CalculateResponsivePanelSizes(availableWidth, availableHeight);

  // Enhanced layout with resizable panels (using responsive member variables)
  float& leftSidebarWidth = currentLeftSidebarWidth;
  float& rightSidebarWidth = currentRightSidebarWidth;
  float& bottomBarHeight = currentBottomBarHeight;

  // Constraints for minimum sizes
  const float minPanelWidth = 200.0f;
  const float minPanelHeight = 100.0f;
  const float splitterThickness = 4.0f;

  // Calculate dynamic sizes
  float usedWidth = 0.0f;
  float usedHeight = 0.0f;

  if (showLeftSidebar) usedWidth += leftSidebarWidth + splitterThickness;
  if (showRightSidebar) usedWidth += rightSidebarWidth + splitterThickness;
  if (showBottomPanel) usedHeight += bottomBarHeight + splitterThickness;

  float centerWidth = availableWidth - usedWidth;
  float mainAreaHeight = availableHeight - usedHeight;

  // Ensure minimum sizes for center panel
  if (centerWidth < minPanelWidth) {
    float excess = minPanelWidth - centerWidth;
    if (showLeftSidebar && showRightSidebar) {
      leftSidebarWidth -= excess * 0.5f;
      rightSidebarWidth -= excess * 0.5f;
    } else if (showLeftSidebar) {
      leftSidebarWidth -= excess;
    } else if (showRightSidebar) {
      rightSidebarWidth -= excess;
    }
    centerWidth = minPanelWidth;
  }

  // Left sidebar with resizable splitter
  if (showLeftSidebar) {
    ImGui::BeginChild("LeftPanel", ImVec2(leftSidebarWidth, mainAreaHeight), true);
    RenderControlPanelContent();
    ImGui::EndChild();

    // Left splitter
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::Colors::GLASS_BORDER);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::Colors::ACCENT_HOVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::Colors::ACCENT_PRIMARY);

    ImGui::Button("##LeftSplitter", ImVec2(splitterThickness, mainAreaHeight));
    if (ImGui::IsItemActive()) {
      leftSidebarWidth += ImGui::GetIO().MouseDelta.x;
      leftSidebarWidth = ImClamp(leftSidebarWidth, minPanelWidth,
                                availableWidth - minPanelWidth - (showRightSidebar ? rightSidebarWidth + splitterThickness : 0));

      // Update percentage when manually resized
      if (usePercentageSizing) {
        leftSidebarWidthPercent = (leftSidebarWidth / availableWidth) * 100.0f;
      }
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
      ImGui::SetTooltip("Drag to resize Control Panel (%.1f%%)\nDouble-click to reset to default size", leftSidebarWidthPercent);
    }
    if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0)) {
      leftSidebarWidthPercent = 20.0f; // Reset to 20%
      usePercentageSizing = true;
    }

    ImGui::PopStyleColor(3);
    ImGui::SameLine();
  }

  // Center visualization area
  ImGui::BeginChild("CenterPanel", ImVec2(centerWidth, mainAreaHeight), true);
  RenderVisualizationContent();
  ImGui::EndChild();

  // Right sidebar with resizable splitter
  if (showRightSidebar) {
    // Right splitter
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::Colors::GLASS_BORDER);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::Colors::ACCENT_HOVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::Colors::ACCENT_PRIMARY);

    ImGui::Button("##RightSplitter", ImVec2(splitterThickness, mainAreaHeight));
    if (ImGui::IsItemActive()) {
      rightSidebarWidth -= ImGui::GetIO().MouseDelta.x;
      rightSidebarWidth = ImClamp(rightSidebarWidth, minPanelWidth,
                                 availableWidth - minPanelWidth - (showLeftSidebar ? leftSidebarWidth + splitterThickness : 0));

      // Update percentage when manually resized (disable auto-percentage mode)
      if (usePercentageSizing) {
        rightSidebarWidthPercent = (rightSidebarWidth / availableWidth) * 100.0f;
      }
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
      ImGui::SetTooltip("Drag to resize Properties Panel (%.1f%%)\nDouble-click to reset to default size", rightSidebarWidthPercent);
    }
    if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0)) {
      rightSidebarWidthPercent = 25.0f; // Reset to 25%
      usePercentageSizing = true;
    }

    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::BeginChild("RightPanel", ImVec2(rightSidebarWidth, mainAreaHeight), true);
    RenderPropertiesPanelContent();
    ImGui::EndChild();
  }

  // Bottom status panel with resizable splitter
  if (showBottomPanel) {
    // Bottom splitter
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::Colors::GLASS_BORDER);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::Colors::ACCENT_HOVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::Colors::ACCENT_PRIMARY);

    ImGui::Button("##BottomSplitter", ImVec2(-1, splitterThickness));
    if (ImGui::IsItemActive()) {
      bottomBarHeight -= ImGui::GetIO().MouseDelta.y;
      bottomBarHeight = ImClamp(bottomBarHeight, minPanelHeight, availableHeight * 0.6f);

      // Update percentage when manually resized
      if (usePercentageSizing) {
        bottomPanelHeightPercent = (bottomBarHeight / availableHeight) * 100.0f;
      }
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
      ImGui::SetTooltip("Drag to resize Status Panel (%.1f%%)\nDouble-click to reset to default size", bottomPanelHeightPercent);
    }
    if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0)) {
      bottomPanelHeightPercent = 30.0f; // Reset to 30%
      usePercentageSizing = true;
    }

    ImGui::PopStyleColor(3);

    ImGui::BeginChild("BottomPanel", ImVec2(-1, bottomBarHeight), true);
    RenderStatusPanelContent();
    ImGui::EndChild();
  }

  ImGui::End();

  // Render theme change notification with animation
  if (showThemeNotification) {
    float animationProgress = 1.0f - (themeNotificationTimer / 3.0f);
    float slideOffset = ImGui::Animations::Instance().EaseInOutCubic(std::min(animationProgress * 2.0f, 1.0f));
    float fadeAlpha = themeNotificationTimer < 0.5f ? (themeNotificationTimer / 0.5f) : 1.0f;

    float yPos = 30 + (1.0f - slideOffset) * -50; // Slide down animation
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, yPos), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.9f * fadeAlpha);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(25, 15));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::Colors::ACCENT_PRIMARY);

    if (ImGui::Begin("ThemeNotification", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs)) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImGui::Colors::ACCENT_PRIMARY);
      ImGui::Text("🎨 Theme changed to: %s", currentThemeName.c_str());
      ImGui::PopStyleColor();

      // Add a subtle progress bar showing notification timeout
      ImGui::Spacing();
      float progress = 1.0f - (themeNotificationTimer / 3.0f);
      ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImGui::Colors::ACCENT_SECONDARY);
      ImGui::ProgressBar(progress, ImVec2(-1, 3), "");
      ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
  }

  // Render modal dialogs
  if (showSettings) RenderSettingsModal();
  if (showAbout) RenderAboutModal();
}

void Gui::RenderMenuBar() {
  if (ImGui::BeginMainMenuBar()) {
    ImGuiIO& io = ImGui::GetIO();

    // Enhanced title with gradient background
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::Colors::ACCENT_PRIMARY);
    ImGui::Text("Sine Wave Pro");
    ImGui::PopStyleColor();

    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New Project", "Ctrl+N")) {}
      if (ImGui::MenuItem("Open Project", "Ctrl+O")) {}
      ImGui::Separator();
      if (ImGui::MenuItem("Export Data", "Ctrl+E")) {}
      if (ImGui::MenuItem("Export Image", "Ctrl+Shift+E")) {}
      ImGui::Separator();
      if (ImGui::MenuItem("Exit", "Alt+F4")) {
        running = false;
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
      ImGui::MenuItem("Control Panel", nullptr, &showLeftSidebar);
      ImGui::MenuItem("Properties", nullptr, &showRightSidebar);
      ImGui::MenuItem("Status Panel", nullptr, &showBottomPanel);
      ImGui::Separator();
      if (ImGui::MenuItem("Reset Panel Sizes")) {
        ResetPanelSizes();
      }
      if (ImGui::MenuItem("Save Layout")) {
        SavePanelLayout();
      }
      if (ImGui::MenuItem("Load Layout")) {
        LoadPanelLayout();
      }
      ImGui::Separator();

      // Theme submenu
      if (ImGui::BeginMenu("Theme")) {
        const char* themeNames[] = {"Ocean Blue", "Sunset Orange", "Electric Lime", "Romantic Rose", "Forest Emerald", "Deep Indigo"};
        for (int i = 0; i < ImGui::Themes::GetThemeCount(); i++) {
          if (ImGui::MenuItem(themeNames[i], nullptr, currentThemeIndex == i)) {
            ApplyTheme(i);
            SaveThemePreference();
          }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Reset to Default")) {
          ResetThemeToDefault();
        }
        ImGui::Separator();
        ImGui::MenuItem("Next Theme", "Ctrl+T", false, false);
        ImGui::MenuItem("Previous Theme", "Ctrl+Shift+T", false, false);
        ImGui::EndMenu();
      }

      ImGui::Separator();

      // Panel sizing mode
      if (ImGui::BeginMenu("Panel Sizing")) {
        if (ImGui::MenuItem("Responsive (Percentage)", nullptr, usePercentageSizing)) {
          usePercentageSizing = true;
        }
        if (ImGui::MenuItem("Fixed (Pixels)", nullptr, !usePercentageSizing)) {
          usePercentageSizing = false;
        }
        ImGui::Separator();

        if (usePercentageSizing) {
          ImGui::Text("Current Panel Sizes:");
          ImGui::Text("Left Sidebar: %.1f%%", leftSidebarWidthPercent);
          ImGui::Text("Right Sidebar: %.1f%%", rightSidebarWidthPercent);
          ImGui::Text("Bottom Panel: %.1f%%", bottomPanelHeightPercent);

          ImGui::Separator();
          if (ImGui::MenuItem("Set Right Sidebar to 20%")) {
            SetPanelWidthPercent(leftSidebarWidthPercent, 20.0f);
          }
          if (ImGui::MenuItem("Set Right Sidebar to 25%")) {
            SetPanelWidthPercent(leftSidebarWidthPercent, 25.0f);
          }
          if (ImGui::MenuItem("Set Right Sidebar to 30%")) {
            SetPanelWidthPercent(leftSidebarWidthPercent, 30.0f);
          }
        }
        ImGui::EndMenu();
      }

      ImGui::Separator();
      ImGui::MenuItem("Enable Animations", nullptr, &enableAnimations);
      ImGui::MenuItem("Glass Effects", nullptr, &enableGlassEffect);
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Tools")) {
      if (ImGui::MenuItem("Settings", "Ctrl+,")) {
        showSettings = true;
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Reset View")) {
        waveformZoom = 1.0f;
        waveformOffset = 0.0f;
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
      if (ImGui::MenuItem("About")) {
        showAbout = true;
      }
      if (ImGui::MenuItem("Documentation")) {}
      ImGui::EndMenu();
    }

    // Status indicators
    float pulse = enableAnimations ? ImGui::Animations::Instance().PulseAnimation(2.0f, 0.7f, 1.0f) : 1.0f;
    ImVec4 statusColor = paused ? ImGui::Colors::WARNING : ImGui::Colors::SUCCESS;
    statusColor.w *= pulse;

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::PushStyleColor(ImGuiCol_Text, statusColor);
    ImGui::Text("%s", paused ? "PAUSED" : "RUNNING");
    ImGui::PopStyleColor();

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::Text("FPS: %.1f", io.Framerate);

    ImGui::EndMainMenuBar();
  }
}

void Gui::RenderControlPanelContent() {
  ImGui::PushStyleColor(ImGuiCol_Text, ImGui::Colors::ACCENT_PRIMARY);
  ImGui::Text("Control Panel");
  ImGui::PopStyleColor();

  ImGui::Separator();
  ImGui::Spacing();

  // Play/Pause button with enhanced styling
  ImVec2 buttonSize = ImVec2(ImGui::GetContentRegionAvail().x, 50);
  if (ImGui::GradientButton(paused ? "Resume Simulation" : "Pause Simulation", buttonSize)) {
    TogglePause();
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Wave parameters section
  ImGui::PushStyleColor(ImGuiCol_Text, ImGui::Colors::TEXT_PRIMARY);
  ImGui::Text("Wave Parameters");
  ImGui::PopStyleColor();
  ImGui::Spacing();

  // Frequency control
  ImGui::Text("Frequency (Hz)");
  ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImGui::Colors::ACCENT_PRIMARY);
  ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImGui::Colors::ACCENT_SECONDARY);

  float freq = core_logic_.GetFrequency();
  if (ImGui::SliderFloat("##Frequency", &freq, 0.1f, 100.0f, "%.2f Hz")) {
    core_logic_.GetFrequency() = freq;
  }
  ImGui::PopStyleColor(2);

  ImGui::Spacing();

  // Amplitude control
  ImGui::Text("Amplitude");
  ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImGui::Colors::SUCCESS);

  float amp = core_logic_.GetAmplitude();
  if (ImGui::SliderFloat("##Amplitude", &amp, 0.1f, 10.0f, "%.2f")) {
    core_logic_.GetAmplitude() = amp;
  }
  ImGui::PopStyleColor();

  ImGui::Spacing();

  // FPS control
  ImGui::Text("Update Rate (FPS)");
  ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImGui::Colors::WARNING);

  float fps = core_logic_.GetFps();
  if (ImGui::SliderFloat("##FPS", &fps, 5.0f, 600.0f, "%.0f FPS")) {
    core_logic_.GetFps() = fps;
  }
  ImGui::PopStyleColor();

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Quick presets section
  ImGui::Text("Quick Presets");
  ImGui::Spacing();

  ImVec2 presetButtonSize = ImVec2((ImGui::GetContentRegionAvail().x - 10) / 2, 0);

  if (ImGui::GradientButton("Low Freq", presetButtonSize)) {
    core_logic_.GetFrequency() = 1.0f;
    core_logic_.GetAmplitude() = 5.0f;
  }
  ImGui::SameLine();
  if (ImGui::GradientButton("High Freq", presetButtonSize)) {
    core_logic_.GetFrequency() = 50.0f;
    core_logic_.GetAmplitude() = 2.0f;
  }

  if (ImGui::GradientButton("Smooth", presetButtonSize)) {
    core_logic_.GetFrequency() = 5.0f;
    core_logic_.GetAmplitude() = 3.0f;
    core_logic_.GetFps() = 60.0f;
  }
  ImGui::SameLine();
  if (ImGui::GradientButton("Chaotic", presetButtonSize)) {
    core_logic_.GetFrequency() = 25.0f;
    core_logic_.GetAmplitude() = 8.0f;
    core_logic_.GetFps() = 120.0f;
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Wave type quick buttons
  ImGui::Text("Wave Type Presets");
  ImGui::Spacing();

  ImVec2 waveButtonSize = ImVec2((ImGui::GetContentRegionAvail().x - 20) / 3, 0);

  if (ImGui::GradientButton("Sine", waveButtonSize)) {
    core_logic_.GetWaveType() = WaveType::SINE;
  }
  ImGui::SameLine();
  if (ImGui::GradientButton("Square", waveButtonSize)) {
    core_logic_.GetWaveType() = WaveType::SQUARE;
  }
  ImGui::SameLine();
  if (ImGui::GradientButton("Triangle", waveButtonSize)) {
    core_logic_.GetWaveType() = WaveType::TRIANGLE;
  }

  if (ImGui::GradientButton("Cosine", waveButtonSize)) {
    core_logic_.GetWaveType() = WaveType::COSINE;
  }
  ImGui::SameLine();
  if (ImGui::GradientButton("Sawtooth", waveButtonSize)) {
    core_logic_.GetWaveType() = WaveType::SAWTOOTH;
  }
}

void Gui::RenderVisualizationContent() {
  ImGui::PushStyleColor(ImGuiCol_Text, ImGui::Colors::ACCENT_PRIMARY);
  ImGui::Text("Wave Visualization");
  ImGui::PopStyleColor();

  // Display current wave parameters
  const char* waveTypeNames[] = {"Sine", "Cosine", "Square", "Triangle", "Sawtooth"};
  int waveTypeIndex = static_cast<int>(core_logic_.GetWaveType());
  ImGui::Text("Type: %s | Freq: %.1f Hz | Amp: %.1f | Phase: %.2f rad",
              waveTypeNames[waveTypeIndex],
              core_logic_.GetFrequency(),
              core_logic_.GetAmplitude(),
              core_logic_.GetPhase());

  // Show noise level if active
  if (core_logic_.GetNoise() > 0.0f) {
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::Colors::WARNING);
    ImGui::Text("| Noise: %.3f", core_logic_.GetNoise());
    ImGui::PopStyleColor();
  }

  ImGui::Separator();
  ImGui::Spacing();

  // Enhanced sine wave plot
  if (!core_logic_.GetSineWaveValues().empty()) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();
    canvas_size.y = std::max(canvas_size.y - 60, 200.0f);

    // Background with grid
    ImVec2 canvas_end = ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y);
    float* bgColor = core_logic_.GetBgColor();
    ImU32 bg_color = ImGui::GetColorU32(ImVec4(bgColor[0], bgColor[1], bgColor[2], 1.0f));
    draw_list->AddRectFilled(canvas_pos, canvas_end, bg_color);

    // Draw grid
    const float grid_size = 50.0f;
    ImU32 grid_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.2f));

    for (float x = canvas_pos.x; x < canvas_end.x; x += grid_size) {
      draw_list->AddLine(ImVec2(x, canvas_pos.y), ImVec2(x, canvas_end.y), grid_color);
    }
    for (float y = canvas_pos.y; y < canvas_end.y; y += grid_size) {
      draw_list->AddLine(ImVec2(canvas_pos.x, y), ImVec2(canvas_end.x, y), grid_color);
    }

    // Draw enhanced sine wave
    const auto& values = core_logic_.GetSineWaveValues();
    const float scale_x = canvas_size.x / static_cast<float>(values.size() - 1);
    const float center_y = canvas_pos.y + canvas_size.y * 0.5f;
    const float scale_y = canvas_size.y * 0.4f / 10.0f;

    // Get wave color from CoreLogic
    float* waveColorArray = core_logic_.GetWaveColor();
    ImVec4 waveColorVec = ImVec4(waveColorArray[0], waveColorArray[1], waveColorArray[2], 1.0f);

    // Draw glow effect if animations are enabled
    if (enableAnimations) {
      for (int pass = 0; pass < 2; pass++) {
        float alpha = (2 - pass) * 0.08f;
        float thickness = 2.0f + pass * 1.5f;
        ImU32 glow_color = ImGui::GetColorU32(ImVec4(
          waveColorVec.x,
          waveColorVec.y,
          waveColorVec.z,
          alpha
        ));

        for (size_t i = 0; i < values.size() - 1; i++) {
          ImVec2 p1(canvas_pos.x + i * scale_x, center_y - values[i] * scale_y);
          ImVec2 p2(canvas_pos.x + (i + 1) * scale_x, center_y - values[i + 1] * scale_y);
          draw_list->AddLine(p1, p2, glow_color, thickness);
        }
      }
    }

    // Draw main wave line using custom wave color
    ImU32 wave_color = ImGui::GetColorU32(waveColorVec);
    for (size_t i = 0; i < values.size() - 1; i++) {
      ImVec2 p1(canvas_pos.x + i * scale_x, center_y - values[i] * scale_y);
      ImVec2 p2(canvas_pos.x + (i + 1) * scale_x, center_y - values[i + 1] * scale_y);
      draw_list->AddLine(p1, p2, wave_color, 2.0f);
    }

    // Draw center line
    draw_list->AddLine(
      ImVec2(canvas_pos.x, center_y),
      ImVec2(canvas_end.x, center_y),
      ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 0.5f)),
      1.0f
    );

    // Draw current value indicator
    if (!values.empty()) {
      float current_value = values.back();
      ImVec2 indicator_pos(canvas_end.x - 20, center_y - current_value * scale_y);

      // Pulsating dot
      if (enableAnimations) {
        float pulse = ImGui::Animations::Instance().PulseAnimation(4.0f, 0.5f, 1.0f);
        ImGui::DrawGlow(draw_list, indicator_pos, 8.0f * pulse, waveColorVec);
      }
      draw_list->AddCircleFilled(indicator_pos, 4.0f,
                                ImGui::GetColorU32(waveColorVec));
    }

    ImGui::InvisibleButton("canvas", canvas_size);
  } else {
    // No data message
    ImVec2 textSize = ImGui::CalcTextSize("No data to display");
    ImVec2 center = ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - textSize.x * 0.5f,
                          ImGui::GetContentRegionAvail().y * 0.5f - textSize.y * 0.5f);
    ImGui::SetCursorPos(center);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::Colors::TEXT_SECONDARY);
    ImGui::Text("No data to display");
    ImGui::PopStyleColor();
  }
}

void Gui::RenderPropertiesPanelContent() {
  ImGui::PushStyleColor(ImGuiCol_Text, ImGui::Colors::ACCENT_PRIMARY);
  ImGui::Text("Properties");
  ImGui::PopStyleColor();

  ImGui::Separator();
  ImGui::Spacing();

  // Tabbed interface
  // Apply theme colors to tabs
  ImGui::PushStyleColor(ImGuiCol_Tab, ImGui::Colors::PRIMARY_MEDIUM);
  ImGui::PushStyleColor(ImGuiCol_TabHovered, ImGui::Colors::ACCENT_HOVER);
  ImGui::PushStyleColor(ImGuiCol_TabActive, ImGui::Colors::ACCENT_PRIMARY);
  ImGui::PushStyleColor(ImGuiCol_TabUnfocused, ImGui::Colors::PRIMARY_DARK);
  ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, ImGui::Colors::ACCENT_SECONDARY);

  if (ImGui::BeginTabBar("PropertiesTabs")) {
    if (ImGui::BeginTabItem("Wave")) {
      ImGui::Spacing();

      // Wave type selection
      ImGui::Text("Wave Type");
      const char* waveTypes[] = {"Sine", "Cosine", "Square", "Triangle", "Sawtooth"};
      int currentWaveType = static_cast<int>(core_logic_.GetWaveType());

      // Apply theme colors to combo box
      PushComboThemeColors();

      if (ImGui::Combo("##WaveType", &currentWaveType, waveTypes, IM_ARRAYSIZE(waveTypes))) {
        core_logic_.GetWaveType() = static_cast<WaveType>(currentWaveType);
      }

      // Pop the combo box colors
      PopThemeColors(9);
      if (ImGui::IsItemHovered()) {
        const char* tooltips[] = {
          "Sine: Classic smooth wave, ideal for pure tones",
          "Cosine: Sine wave shifted by 90 degrees",
          "Square: Digital wave with sharp transitions",
          "Triangle: Linear wave with sharp peaks",
          "Sawtooth: Ramp wave used in synthesizers"
        };
        ImGui::SetTooltip("%s", tooltips[currentWaveType]);
      }

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      // Phase control
      ImGui::Text("Phase Shift");
      float& phase = core_logic_.GetPhase();

      // Apply theme colors to phase slider
      PushSliderThemeColors();

      ImGui::SliderFloat("##Phase", &phase, 0.0f, 6.28f, "%.2f rad");
      PopThemeColors(5);

      // Noise control
      ImGui::Text("Noise Level");
      float& noise = core_logic_.GetNoise();

      // Apply theme colors to noise slider
      PushSliderThemeColors();

      ImGui::SliderFloat("##Noise", &noise, 0.0f, 1.0f, "%.3f");
      PopThemeColors(5);

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Themes")) {
      ImGui::Spacing();

      // Color Theme Selection
      ImGui::Text("Color Theme Selection");
      ImGui::Spacing();

      const auto& themes = ImGui::Themes::themes;
      const int themeCount = ImGui::Themes::GetThemeCount();

      // Theme preview buttons in a grid
      ImVec2 buttonSize = ImVec2(140, 40);
      int buttonsPerRow = 2;
      for (int i = 0; i < themeCount; i++) {
        if (i > 0 && i % buttonsPerRow != 0) ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, themes[i].primary);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, themes[i].hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, themes[i].secondary);

        bool selected = (currentThemeIndex == i);
        if (selected) {
          ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
          ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.8f));
        }

        char buttonId[32];
        sprintf(buttonId, "%s##theme%d", themes[i].name, i);
        if (ImGui::Button(buttonId, buttonSize)) {
          currentThemeIndex = i;
          ImGui::Themes::SetTheme(themes[i]);
          ImGui::Colors::UpdateAccentColors();
          themeChanged = true;
        }

        if (selected) {
          ImGui::PopStyleVar();
          ImGui::PopStyleColor();
        }

        ImGui::PopStyleColor(3);

        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Click to apply %s theme", themes[i].name);
        }
      }

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      // Current theme details and preview
      ImGui::Text("Current Theme: %s", themes[currentThemeIndex].name);
      ImGui::Spacing();

      // Theme color preview
      ImGui::Text("Theme Colors:");
      const auto& currentTheme = themes[currentThemeIndex];

      // Primary color preview
      ImGui::ColorButton("Primary##preview", currentTheme.primary, ImGuiColorEditFlags_NoTooltip, ImVec2(40, 20));
      ImGui::SameLine();
      ImGui::Text("Primary");

      // Secondary color preview
      ImGui::ColorButton("Secondary##preview", currentTheme.secondary, ImGuiColorEditFlags_NoTooltip, ImVec2(40, 20));
      ImGui::SameLine();
      ImGui::Text("Secondary");

      // Hover color preview
      ImGui::ColorButton("Hover##preview", currentTheme.hover, ImGuiColorEditFlags_NoTooltip, ImVec2(40, 20));
      ImGui::SameLine();
      ImGui::Text("Hover");

      ImGui::Spacing();

      // Color information table
      if (ImGui::BeginTable("ColorInfo", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Preview");
        ImGui::TableSetupColumn("RGB Values");
        ImGui::TableSetupColumn("Hex Code");
        ImGui::TableHeadersRow();

        // Primary color row
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Primary");
        ImGui::TableSetColumnIndex(1);
        ImGui::ColorButton("##primary_preview", currentTheme.primary, ImGuiColorEditFlags_NoTooltip, ImVec2(30, 20));
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("R:%.0f G:%.0f B:%.0f", currentTheme.primary.x * 255, currentTheme.primary.y * 255, currentTheme.primary.z * 255);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("#%02X%02X%02X", (int)(currentTheme.primary.x * 255), (int)(currentTheme.primary.y * 255), (int)(currentTheme.primary.z * 255));

        // Secondary color row
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Secondary");
        ImGui::TableSetColumnIndex(1);
        ImGui::ColorButton("##secondary_preview", currentTheme.secondary, ImGuiColorEditFlags_NoTooltip, ImVec2(30, 20));
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("R:%.0f G:%.0f B:%.0f", currentTheme.secondary.x * 255, currentTheme.secondary.y * 255, currentTheme.secondary.z * 255);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("#%02X%02X%02X", (int)(currentTheme.secondary.x * 255), (int)(currentTheme.secondary.y * 255), (int)(currentTheme.secondary.z * 255));

        // Hover color row
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Hover");
        ImGui::TableSetColumnIndex(1);
        ImGui::ColorButton("##hover_preview", currentTheme.hover, ImGuiColorEditFlags_NoTooltip, ImVec2(30, 20));
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("R:%.0f G:%.0f B:%.0f", currentTheme.hover.x * 255, currentTheme.hover.y * 255, currentTheme.hover.z * 255);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("#%02X%02X%02X", (int)(currentTheme.hover.x * 255), (int)(currentTheme.hover.y * 255), (int)(currentTheme.hover.z * 255));

        ImGui::EndTable();
      }

      ImGui::Spacing();
      ImGui::Text("Theme Explanation:");
      ImGui::BulletText("Primary: Main accent color for buttons and highlights");
      ImGui::BulletText("Secondary: Lighter version for gradients and active states");
      ImGui::BulletText("Hover: Medium shade for hover feedback");

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Display")) {
      ImGui::Spacing();

      // Display options
      ImGui::Text("Visual Effects");
      ImGui::Checkbox("Show Grid", &enableGlassEffect);
      ImGui::Checkbox("Enable Glow Effect", &enableAnimations);
      ImGui::Checkbox("Enable Animations", &enableAnimations);
      ImGui::Checkbox("Glass Effects", &enableGlassEffect);

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      // Advanced Color Settings
      ImGui::Text("Custom Colors");
      float* waveColor = core_logic_.GetWaveColor();
      if (ImGui::ColorEdit3("Wave Color##WaveColor", waveColor)) {
        core_logic_.SetWaveColor(waveColor[0], waveColor[1], waveColor[2]);
      }

      float* bgColor = core_logic_.GetBgColor();
      if (ImGui::ColorEdit3("Background Color##BgColor", bgColor)) {
        core_logic_.SetBgColor(bgColor[0], bgColor[1], bgColor[2]);
      }

      ImGui::Spacing();
      if (ImGui::GradientButton("Reset to Theme Colors", ImVec2(-1, 0))) {
        // Reset wave color to current theme accent
        const auto& currentTheme = ImGui::Themes::GetCurrentTheme();
        core_logic_.SetWaveColor(currentTheme.primary.x, currentTheme.primary.y, currentTheme.primary.z);
        // Reset background to default dark
        core_logic_.SetBgColor(0.12f, 0.14f, 0.18f);
      }

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Export")) {
      ImGui::Spacing();

      ImGui::Text("Export Options");
      ImGui::Spacing();

      if (ImGui::GradientButton("Export as PNG", ImVec2(-1, 0))) {
        // Future: implement PNG export
      }

      if (ImGui::GradientButton("Export as CSV", ImVec2(-1, 0))) {
        // Future: implement CSV export
      }

      if (ImGui::GradientButton("Export as WAV", ImVec2(-1, 0))) {
        // Future: implement WAV export
      }

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      ImGui::Text("Settings Export");
      ImGui::Spacing();

      if (ImGui::GradientButton("Export Current Settings", ImVec2(-1, 0))) {
        // Export current wave and color settings
        try {
          std::filesystem::create_directories("exports");
          std::ofstream settingsFile("exports/wave_settings.txt");
          if (settingsFile.is_open()) {
            const char* waveTypeNames[] = {"Sine", "Cosine", "Square", "Triangle", "Sawtooth"};
            int waveTypeIndex = static_cast<int>(core_logic_.GetWaveType());
            float* waveColor = core_logic_.GetWaveColor();
            float* bgColor = core_logic_.GetBgColor();

            settingsFile << "=== Wave Simulator Settings ===" << std::endl;
            settingsFile << "Wave Type: " << waveTypeNames[waveTypeIndex] << std::endl;
            settingsFile << "Frequency: " << core_logic_.GetFrequency() << " Hz" << std::endl;
            settingsFile << "Amplitude: " << core_logic_.GetAmplitude() << std::endl;
            settingsFile << "Phase: " << core_logic_.GetPhase() << " rad" << std::endl;
            settingsFile << "Noise: " << core_logic_.GetNoise() << std::endl;
            settingsFile << "FPS: " << core_logic_.GetFps() << std::endl;
            settingsFile << "Wave Color RGB: " << waveColor[0] << ", " << waveColor[1] << ", " << waveColor[2] << std::endl;
            settingsFile << "Background Color RGB: " << bgColor[0] << ", " << bgColor[1] << ", " << bgColor[2] << std::endl;
            settingsFile << "Theme: " << currentThemeName << std::endl;
            settingsFile.close();
          }
        } catch (const std::exception& e) {
          fmt::print("Failed to export settings: {}\n", e.what());
        }
      }

      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }

  // Pop the tab styling colors
  ImGui::PopStyleColor(5);
}

void Gui::RenderStatusPanelContent() {
  ImGui::PushStyleColor(ImGuiCol_Text, ImGui::Colors::ACCENT_PRIMARY);
  ImGui::Text("Status & Analytics");
  ImGui::PopStyleColor();

  ImGui::Separator();
  ImGui::Spacing();

  ImGui::Columns(4, "StatusCols", true);

  // Performance metrics
  ImGui::Text("Performance");
  ImGui::Separator();
  ImGui::Text("Frame Rate: %.1f FPS", ImGui::GetIO().Framerate);
  ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

  // Memory usage approximation
  size_t dataPoints = core_logic_.GetSineWaveValues().size();
  float memoryUsage = dataPoints * sizeof(float) / 1024.0f; // KB
  ImGui::Text("Memory: %.1f KB", memoryUsage);

  ImGui::NextColumn();

  // Wave statistics
  ImGui::Text("Wave Analysis");
  ImGui::Separator();
  if (!core_logic_.GetSineWaveValues().empty()) {
    const auto& values = core_logic_.GetSineWaveValues();

    float minVal = *std::min_element(values.begin(), values.end());
    float maxVal = *std::max_element(values.begin(), values.end());
    float avgVal = std::accumulate(values.begin(), values.end(), 0.0f) / values.size();

    ImGui::Text("Min: %.3f", minVal);
    ImGui::Text("Max: %.3f", maxVal);
    ImGui::Text("Avg: %.3f", avgVal);
    ImGui::Text("Range: %.3f", maxVal - minVal);

    // Current wave parameters
    const char* waveTypeNames[] = {"Sine", "Cosine", "Square", "Triangle", "Sawtooth"};
    int waveTypeIndex = static_cast<int>(core_logic_.GetWaveType());
    ImGui::Text("Type: %s", waveTypeNames[waveTypeIndex]);
    ImGui::Text("Phase: %.2f rad", core_logic_.GetPhase());
    if (core_logic_.GetNoise() > 0.0f) {
      ImGui::Text("Noise: %.3f", core_logic_.GetNoise());
    }
  }

  ImGui::NextColumn();

  // System info
  ImGui::Text("System Info");
  ImGui::Separator();
  ImGui::Text("Renderer: SDL2");
  ImGui::Text("UI: ImGui %.2s", ImGui::GetVersion());
  ImGui::Text("Samples: %zu", core_logic_.GetSineWaveValues().size());

  ImGui::NextColumn();

  // Quick actions
  ImGui::Text("Quick Actions & Layout");
  ImGui::Separator();

  if (ImGui::GradientButton("Clear Data", ImVec2(-1, 0))) {
    // Future: implement data clearing
  }

  if (ImGui::GradientButton("Screenshot", ImVec2(-1, 0))) {
    // Future: implement screenshot
  }

  if (ImGui::GradientButton("Reset All", ImVec2(-1, 0))) {
    core_logic_.GetFrequency() = 5.0f;
    core_logic_.GetAmplitude() = 5.0f;
    core_logic_.GetFps() = 60.0f;
  }

  ImGui::Spacing();
  ImGui::Text("Panel Layout");
  ImGui::Separator();

  // Display current panel sizing mode
  if (usePercentageSizing) {
    ImGui::Text("Mode: Responsive");
    ImGui::Text("Left: %.1f%%", leftSidebarWidthPercent);
    ImGui::Text("Right: %.1f%%", rightSidebarWidthPercent);
    ImGui::Text("Bottom: %.1f%%", bottomPanelHeightPercent);
  } else {
    ImGui::Text("Mode: Fixed");
    ImGui::Text("Left: %.0fpx", currentLeftSidebarWidth);
    ImGui::Text("Right: %.0fpx", currentRightSidebarWidth);
    ImGui::Text("Bottom: %.0fpx", currentBottomBarHeight);
  }

  ImGui::Columns(1);
}

void Gui::RenderControlPanel() {
  // This method is kept for compatibility but content moved to RenderControlPanelContent
}

void Gui::RenderVisualizationPanel() {
  // This method is kept for compatibility but content moved to RenderVisualizationContent
}

void Gui::RenderPropertiesPanel() {
  // This method is kept for compatibility but content moved to RenderPropertiesPanelContent
}

void Gui::RenderStatusPanel() {
  // This method is kept for compatibility but content moved to RenderStatusPanelContent
}

void Gui::ResetPanelSizes() {
  // Reset to default percentages for responsive sizing
  leftSidebarWidthPercent = 20.0f;   // 20% of width
  rightSidebarWidthPercent = 25.0f;  // 25% of width
  bottomPanelHeightPercent = 30.0f;  // 30% of height

  usePercentageSizing = true;

  showLeftSidebar = true;
  showRightSidebar = true;
  showBottomPanel = true;
}

void Gui::SavePanelLayout() {
  // In a real application, you might save to a config file
  // For now, we'll just store defaults
}

void Gui::LoadPanelLayout() {
  // In a real application, you might load from a config file
  // For now, we'll just use defaults
}

// Theme Management Implementation
// ===============================
// This section provides complete theme management functionality including:
// - Theme persistence across application runs
// - Keyboard shortcuts (Ctrl+T for next, Ctrl+Shift+T for previous)
// - Visual notifications with animations
// - Theme preview in properties panel
// - Menu-based theme selection
// - Reset to default functionality
//
// Available Themes:
// - Ocean Blue (default)
// - Sunset Orange
// - Electric Lime
// - Romantic Rose
// - Forest Emerald
// - Deep Indigo

void Gui::SaveThemePreference() {
  try {
    std::filesystem::create_directories("config");
    std::ofstream configFile("config/theme.cfg");
    if (configFile.is_open()) {
      configFile << "current_theme=" << currentThemeIndex << std::endl;
      configFile << "theme_name=" << currentThemeName << std::endl;
      configFile.close();
    }
  } catch (const std::exception& e) {
    fmt::print("Failed to save theme preference: {}\n", e.what());
  }
}

void Gui::LoadThemePreference() {
  try {
    std::ifstream configFile("config/theme.cfg");
    if (configFile.is_open()) {
      std::string line;
      while (std::getline(configFile, line)) {
        if (line.find("current_theme=") == 0) {
          int themeIndex = std::stoi(line.substr(14));
          if (themeIndex >= 0 && themeIndex < ImGui::Themes::GetThemeCount()) {
            currentThemeIndex = themeIndex;
          } else {
            // Handle removed themes - fallback to default
            fmt::print("Saved theme index {} is out of range, falling back to default\n", themeIndex);
            currentThemeIndex = 0;
          }
        } else if (line.find("theme_name=") == 0) {
          std::string savedThemeName = line.substr(11);
          // Check if the saved theme name is one of the removed themes
          if (savedThemeName == "Royal Purple" || savedThemeName == "Golden Amber") {
            fmt::print("Saved theme '{}' has been removed, falling back to default\n", savedThemeName);
            currentThemeIndex = 0;
            currentThemeName = "Ocean Blue";
          } else {
            currentThemeName = savedThemeName;
          }
        }
      }
      configFile.close();
    }
  } catch (const std::exception& e) {
    fmt::print("Failed to load theme preference: {}\n", e.what());
  }

  // Apply the loaded (or default) theme
  ApplyTheme(currentThemeIndex);
}

void Gui::ApplyTheme(int themeIndex) {
  // Apply theme with bounds checking and visual feedback
  if (themeIndex >= 0 && themeIndex < ImGui::Themes::GetThemeCount()) {
    currentThemeIndex = themeIndex;
    ImGui::Themes::SetTheme(ImGui::Themes::themes[currentThemeIndex]);
    ImGui::Colors::UpdateAccentColors();
    themeChanged = false;

    // Auto-sync wave color with new theme
    const auto& newTheme = ImGui::Themes::themes[currentThemeIndex];
    core_logic_.SetWaveColor(newTheme.primary.x, newTheme.primary.y, newTheme.primary.z);

    // Show animated theme change notification
    currentThemeName = ImGui::Themes::themes[currentThemeIndex].name;
    showThemeNotification = true;
    themeNotificationTimer = 3.0f; // Show for 3 seconds
  }
}

void Gui::ResetThemeToDefault() {
  // Reset to default Ocean Blue theme and save preference
  ApplyTheme(0); // Reset to Ocean Blue theme (index 0)
  SaveThemePreference();
}

// Responsive Panel Sizing Implementation
// ======================================
// Methods to handle percentage-based responsive panel sizing

void Gui::CalculateResponsivePanelSizes(float availableWidth, float availableHeight) {
  if (usePercentageSizing) {
    // Calculate panel sizes based on percentages
    currentLeftSidebarWidth = availableWidth * (leftSidebarWidthPercent / 100.0f);
    currentRightSidebarWidth = availableWidth * (rightSidebarWidthPercent / 100.0f);
    currentBottomBarHeight = availableHeight * (bottomPanelHeightPercent / 100.0f);

    // Apply minimum constraints
    const float minPanelWidth = 200.0f;
    const float minPanelHeight = 100.0f;

    currentLeftSidebarWidth = std::max(currentLeftSidebarWidth, minPanelWidth);
    currentRightSidebarWidth = std::max(currentRightSidebarWidth, minPanelWidth);
    currentBottomBarHeight = std::max(currentBottomBarHeight, minPanelHeight);

    // Ensure total width doesn't exceed available space
    float totalSidebarWidth = currentLeftSidebarWidth + currentRightSidebarWidth;
    if (totalSidebarWidth > availableWidth * 0.8f) { // Leave at least 20% for center
      float scale = (availableWidth * 0.8f) / totalSidebarWidth;
      currentLeftSidebarWidth *= scale;
      currentRightSidebarWidth *= scale;
    }
  }
}

void Gui::SetPanelWidthPercent(float leftPercent, float rightPercent) {
  leftSidebarWidthPercent = std::clamp(leftPercent, 10.0f, 40.0f);
  rightSidebarWidthPercent = std::clamp(rightPercent, 10.0f, 40.0f);

  // Ensure total doesn't exceed 70% to leave space for center panel
  if (leftSidebarWidthPercent + rightSidebarWidthPercent > 70.0f) {
    float total = leftSidebarWidthPercent + rightSidebarWidthPercent;
    leftSidebarWidthPercent = (leftSidebarWidthPercent / total) * 70.0f;
    rightSidebarWidthPercent = (rightSidebarWidthPercent / total) * 70.0f;
  }
}

void Gui::SetBottomPanelHeightPercent(float heightPercent) {
  bottomPanelHeightPercent = std::clamp(heightPercent, 10.0f, 50.0f);
}

std::string Gui::GetPanelSizeInfo() const {
  if (usePercentageSizing) {
    return fmt::format("Responsive Mode - Left: {:.1f}%, Right: {:.1f}%, Bottom: {:.1f}%",
                       leftSidebarWidthPercent, rightSidebarWidthPercent, bottomPanelHeightPercent);
  } else {
    return fmt::format("Fixed Mode - Left: {:.0f}px, Right: {:.0f}px, Bottom: {:.0f}px",
                       currentLeftSidebarWidth, currentRightSidebarWidth, currentBottomBarHeight);
  }
}

// UI Theme Helper Methods
// =======================

void Gui::PushComboThemeColors() {
  // Apply theme colors to combo box elements (9 colors total):
  // - Frame background colors for combo box itself
  ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::Colors::PRIMARY_MEDIUM);
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImGui::Colors::ACCENT_HOVER);
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImGui::Colors::ACCENT_SECONDARY);
  // - Dropdown popup background
  ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::Colors::PRIMARY_DARK);
  // - Dropdown item colors
  ImGui::PushStyleColor(ImGuiCol_Header, ImGui::Colors::ACCENT_SECONDARY);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::Colors::ACCENT_HOVER);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::Colors::ACCENT_PRIMARY);
  // - Dropdown arrow button colors, use ImGuiCol_Text for the down arrow color
  ImGui::PushStyleColor(ImGuiCol_Button, ImGui::Colors::ACCENT_PRIMARY);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::Colors::ACCENT_HOVER);
}

void Gui::PushSliderThemeColors() {
  ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::Colors::PRIMARY_MEDIUM);
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImGui::Colors::ACCENT_HOVER);
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImGui::Colors::ACCENT_SECONDARY);
  ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImGui::Colors::ACCENT_PRIMARY);
  ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImGui::Colors::ACCENT_HOVER);
}

void Gui::PopThemeColors(int count) {
  ImGui::PopStyleColor(count);
}





void Gui::RenderSettingsModal() {
  if (showSettings && !ImGui::IsPopupOpen("Settings")) {
    ImGui::OpenPopup("Settings");
  }

  ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

  if (ImGui::BeginPopupModal("Settings", &showSettings, ImGuiWindowFlags_NoResize)) {
    ImGui::Text("⚙️ Application Settings");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("SettingsTabs")) {
      if (ImGui::BeginTabItem("General")) {
        ImGui::Checkbox("Enable Animations", &enableAnimations);
        ImGui::Checkbox("Glass Effect", &enableGlassEffect);

        ImGui::Text("Theme");
        const char* themeNames[] = {"Ocean Blue", "Sunset Orange", "Electric Lime", "Romantic Rose", "Forest Emerald", "Deep Indigo"};

        // Apply theme colors to settings theme combo box
        PushComboThemeColors();

        if (ImGui::Combo("##Theme", &currentThemeIndex, themeNames, IM_ARRAYSIZE(themeNames))) {
          ImGui::Themes::SetTheme(ImGui::Themes::themes[currentThemeIndex]);
          ImGui::Colors::UpdateAccentColors();
          themeChanged = true;
        }

        // Pop the combo box colors
        PopThemeColors(9);

        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Performance")) {
        ImGui::Text("Rendering Settings");

        static bool vsync = true;
        ImGui::Checkbox("V-Sync", &vsync);

        static int maxFPS = 120;
        ImGui::SliderInt("Max FPS", &maxFPS, 30, 240);

        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Panel Layout")) {
        ImGui::Text("Panel Sizing Configuration");
        ImGui::Spacing();

        ImGui::Checkbox("Use Responsive Sizing (Percentage)", &usePercentageSizing);
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("When enabled, panels resize based on window size percentage.\nWhen disabled, panels use fixed pixel sizes.");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (usePercentageSizing) {
          ImGui::Text("Sidebar Widths (as percentage of window width):");

          float tempLeft = leftSidebarWidthPercent;
          PushSliderThemeColors();
          if (ImGui::SliderFloat("Left Sidebar Width", &tempLeft, 10.0f, 40.0f, "%.1f%%")) {
            SetPanelWidthPercent(tempLeft, rightSidebarWidthPercent);
          }
          PopThemeColors(5);

          float tempRight = rightSidebarWidthPercent;
          PushSliderThemeColors();
          if (ImGui::SliderFloat("Right Sidebar Width", &tempRight, 10.0f, 40.0f, "%.1f%%")) {
            SetPanelWidthPercent(leftSidebarWidthPercent, tempRight);
          }
          PopThemeColors(5);

          ImGui::Spacing();
          ImGui::Text("Bottom Panel Height (as percentage of window height):");

          float tempBottom = bottomPanelHeightPercent;
          PushSliderThemeColors();
          if (ImGui::SliderFloat("Bottom Panel Height", &tempBottom, 10.0f, 50.0f, "%.1f%%")) {
            SetBottomPanelHeightPercent(tempBottom);
          }
          PopThemeColors(5);

          ImGui::Spacing();
          ImGui::Text("Quick Presets:");
          if (ImGui::Button("Balanced (20% | 25% | 30%)")) {
            SetPanelWidthPercent(20.0f, 25.0f);
            SetBottomPanelHeightPercent(30.0f);
          }
          ImGui::SameLine();
          if (ImGui::Button("Wide Right (15% | 35% | 25%)")) {
            SetPanelWidthPercent(15.0f, 35.0f);
            SetBottomPanelHeightPercent(25.0f);
          }
          if (ImGui::Button("Compact (15% | 20% | 20%)")) {
            SetPanelWidthPercent(15.0f, 20.0f);
            SetBottomPanelHeightPercent(20.0f);
          }
        } else {
          ImGui::Text("Fixed Panel Sizes (in pixels):");
          PushSliderThemeColors();
          ImGui::SliderFloat("Left Sidebar Width", &currentLeftSidebarWidth, 200.0f, 500.0f, "%.0f px");
          PopThemeColors(5);
          PushSliderThemeColors();
          ImGui::SliderFloat("Right Sidebar Width", &currentRightSidebarWidth, 200.0f, 500.0f, "%.0f px");
          PopThemeColors(5);
          PushSliderThemeColors();
          ImGui::SliderFloat("Bottom Panel Height", &currentBottomBarHeight, 100.0f, 400.0f, "%.0f px");
          PopThemeColors(5);
        }

        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }

    ImGui::Spacing();
    ImGui::Separator();

    float buttonWidth = 100;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth - 20);

    if (ImGui::GradientButton("Close", ImVec2(buttonWidth, 0))) {
      showSettings = false;
    }

    ImGui::EndPopup();
  }
}

void Gui::RenderAboutModal() {
  if (showAbout && !ImGui::IsPopupOpen("About")) {
    ImGui::OpenPopup("About");
  }

  ImGui::SetNextWindowSize(ImVec2(450, 300), ImGuiCond_FirstUseEver);

  if (ImGui::BeginPopupModal("About", &showAbout, ImGuiWindowFlags_NoResize)) {
    // Centered title with animation
    float windowWidth = ImGui::GetWindowSize().x;
    ImVec2 titleSize = ImGui::CalcTextSize("Wave Simulator Pro");
    ImGui::SetCursorPosX((windowWidth - titleSize.x) * 0.5f);

    float pulse = enableAnimations ? ImGui::Animations::Instance().PulseAnimation(1.5f, 0.8f, 1.2f) : 1.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, pulse);
    ImGui::Text("Wave Simulator Pro");
    ImGui::PopStyleVar();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Version: 2.0.0");
    ImGui::Text("Build: %s %s", __DATE__, __TIME__);
    ImGui::Spacing();

    ImGui::Text("A professional sine wave visualization tool");
    ImGui::Text("Built with ImGui, SDL2, and modern C++");
    ImGui::Spacing();

    ImGui::Text("Features:");
    ImGui::BulletText("Real-time wave generation");
    ImGui::BulletText("Interactive controls");
    ImGui::BulletText("Modern UI with animations");
    ImGui::BulletText("Professional styling");

    ImGui::Spacing();
    ImGui::Separator();

    float buttonWidth = 100;
    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

    if (ImGui::GradientButton("Close", ImVec2(buttonWidth, 0))) {
      showAbout = false;
    }

    ImGui::EndPopup();
  }
}

void Gui::Run() {
  ProcessEvents();
  Update();
  Render();
}

void Gui::Cleanup() {
  fmt::print("Starting cleanup...\n");

  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  if (renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;
  }
  if (window) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }
  SDL_Quit();

  fmt::print(fg(fmt::color::green), "Cleanup complete!\n");
}
