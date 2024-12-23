#pragma once
#include <vector>
#ifdef __EMSCRIPTEN__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

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
