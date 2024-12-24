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

  inline bool IsRunning() const { return running; }
  inline bool IsPaused() const { return paused; }
  inline void LoadSystemFonts() {
    ImGuiIO& io = ImGui::GetIO();

    // For native platforms, load system fonts
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
  // Reference for depencency injection
  CoreLogic& core_logic_;
};
