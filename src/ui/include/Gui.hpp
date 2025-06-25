#pragma once
#include <string>
#include <vector>
#ifdef __EMSCRIPTEN__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include "CoreLogic.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"


#ifdef _WIN32
// Windows
constexpr std::string_view defaultFontPath =
    "C:\\Windows\\Fonts\\segoeui.ttf";  // Example font on Windows
#elif __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_MAC
// macOS
constexpr std::string_view defaultFontPath =
    "/System/Library/Fonts/Supplemental/Arial.ttf";  // Example for macOS
#endif
#elif __linux__
// Linux (Ubuntu / CachyOS)
constexpr std::string_view defaultFontPath =
    "/usr/share/fonts/noto/NotoSans-Regular.ttf";
#elif __EMSCRIPTEN__
constexpr std::string_view defaultFontPath = "/fonts//NotoSans-Regular.ttf";
#else
// Default fallback
constexpr std::string_view defaultFontPath = "path/to/default/font.ttf";
#endif

constexpr float FONT_SIZE = 24.f;

class Gui {
 public:
  Gui(CoreLogic& coreLogic);
  ~Gui();

  bool Initialize();
  void Run();
  void Cleanup();
  // Make these public for Emscripten main loop
  void ProcessEvents();
  void Update();
  void Render();
  void TogglePause() { paused = !paused; };

  // New UI rendering methods
  void RenderMainInterface();
  void RenderMenuBar();
  void RenderControlPanel();
  void RenderVisualizationPanel();
  void RenderPropertiesPanel();
  void RenderStatusPanel();
  void RenderSettingsModal();
  void RenderAboutModal();

  // Content rendering methods
  void RenderControlPanelContent();
  void RenderVisualizationContent();
  void RenderPropertiesPanelContent();
  void RenderStatusPanelContent();

  // Panel management methods
  void ResetPanelSizes();
  void SavePanelLayout();
  void LoadPanelLayout();

  // Theme management methods
  void SaveThemePreference();
  void LoadThemePreference();
  void ApplyTheme(int themeIndex);
  void ResetThemeToDefault();

  // Responsive panel sizing methods
  void CalculateResponsivePanelSizes(float availableWidth, float availableHeight);
  void SetPanelWidthPercent(float leftPercent, float rightPercent);
  void SetBottomPanelHeightPercent(float heightPercent);
  std::string GetPanelSizeInfo() const;

  // UI theme helper methods
  void PushComboThemeColors();
  void PushSliderThemeColors();
  void PopThemeColors(int count);

  inline bool IsRunning() const { return running; }
  inline bool IsPaused() const { return paused; }
  inline void LoadSystemFonts() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF(defaultFontPath.data(), FONT_SIZE);
  }

 private:
  SDL_Window* window;
  SDL_Renderer* renderer;
  bool running;
  bool paused;

  std::vector<float> values;

  bool showLeftSidebar = true;
  bool showRightSidebar = true;
  bool showBottomPanel = true;

  // Animation and visual enhancement variables
  float animationTime = 0.0f;
  float sidebarAnimationOffset = 0.0f;
  bool isInitialAnimationComplete = false;

  // UI state variables
  bool showSettings = false;
  bool showAbout = false;
  int selectedTab = 0;
  float waveformZoom = 1.0f;
  float waveformOffset = 0.0f;

  // Visual effect variables
  float glowIntensity = 1.0f;
  bool enableAnimations = true;
  bool enableGlassEffect = true;

  // Theme management variables
  int currentThemeIndex = 0;
  bool themeChanged = false;

  // Theme notification variables
  bool showThemeNotification = false;
  float themeNotificationTimer = 0.0f;
  std::string currentThemeName = "Ocean Blue";

  // Panel size variables (persistent across frames)
  float defaultLeftSidebarWidth = 320.0f;
  float defaultRightSidebarWidth = 300.0f;
  float defaultBottomBarHeight = 150.0f;

  // Current panel sizes (member variables for proper reset)
  float currentLeftSidebarWidth = 320.0f;
  float currentRightSidebarWidth = 300.0f;
  float currentBottomBarHeight = 300.0f;

  // Percentage-based panel sizing (responsive)
  bool usePercentageSizing = true;
  float leftSidebarWidthPercent = 15.0f;    // 20% of available width
  float rightSidebarWidthPercent = 20.0f;   // 25% of available width
  float bottomPanelHeightPercent = 25.0f;   // 30% of available height

  // Reference for depencency injection
  CoreLogic& core_logic_;
};
