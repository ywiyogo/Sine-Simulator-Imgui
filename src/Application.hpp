#pragma once
#include <string>
#include <vector>
#ifdef __EMSCRIPTEN__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

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
    "/usr/share/fonts/noto/NotoSans-Regular.ttf";  // Example for Ubuntu
#else
// Default fallback
constexpr std::string_view defaultFontPath = "path/to/default/font.ttf";
#endif

constexpr float FONT_SIZE = 32.f;

class Application {
 public:
  Application();
  ~Application();

  bool initialize();
  void run();
  void cleanup();
  // Make these public for Emscripten main loop
  void processEvents();
  void update();
  void render();

  inline void LoadSystemFonts() {
    ImGuiIO& io = ImGui::GetIO();

#ifdef __EMSCRIPTEN__
    // For web (Emscripten), load font via Google Fonts or CSS (web-safe font)
    io.Fonts->AddFontDefault();
#else
    // For native platforms, load system fonts
    io.Fonts->AddFontFromFileTTF(defaultFontPath.data(), FONT_SIZE);
#endif
  }

 private:
  SDL_Window* window;
  SDL_Renderer* renderer;
  bool running;

  // Simulation parameters
  float frequency;
  float amplitude;
  float time;
  std::vector<float> values;
  static constexpr size_t MAX_VALUES = 100;
  bool showLeftSidebar = true;
  bool showRightSidebar = true;
  bool showBottomPanel = true;
};
