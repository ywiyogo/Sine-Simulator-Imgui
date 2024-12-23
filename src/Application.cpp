#include "Application.hpp"

#include <cmath>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#define FMT_CONSTEVAL
#endif
// avoid using FMT_STRING macros from fmt/format.h and fmt::println
#include <fmt/color.h>
#include <fmt/core.h>

Application::Application()
    : window(nullptr),
      renderer(nullptr),
      running(true),
      frequency(1.0f),
      amplitude(1.0f),
      time(0.0f) {
  values.reserve(MAX_VALUES);
}

Application::~Application() { cleanup(); }

bool Application::initialize() {
  fmt::print("Initializing SDL...\n");
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fmt::print("SDL initialization failed: {}\n", SDL_GetError());
    return false;
  }

  fmt::print("Creating window...\n");
  window = SDL_CreateWindow("Sine Wave Simulator", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 800, 600,
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

  if (!ImGui_ImplSDL2_InitForSDLRenderer(window, renderer)) {
    fmt::print("ImGui SDL2 initialization failed\n");
    return false;
  }

  if (!ImGui_ImplSDLRenderer2_Init(renderer)) {
    fmt::print("ImGui SDL2 Renderer initialization failed\n");
    return false;
  }

  fmt::print(fg(fmt::color::green), "Initialization complete!\n");
  return true;
}

void Application::processEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT) {
      running = false;
    }
  }
}

void Application::update() {
  time += 0.016f;  // Assuming ~60 FPS
  float value = amplitude * std::sin(2.0f * M_PI * frequency * time);

  values.push_back(value);
  if (values.size() > MAX_VALUES) {
    values.erase(values.begin());
  }
}

void Application::render() {
  SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
  if (SDL_RenderClear(renderer) != 0) {
    fmt::print("Render clear failed: {}\n", SDL_GetError());
    return;
  }

  // Start ImGui frame
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  try {
    // Create ImGui window
    ImGui::Begin("Sine Wave Controls");
    ImGui::SliderFloat("Frequency", &frequency, 0.1f, 5.0f);
    ImGui::SliderFloat("Amplitude", &amplitude, 0.1f, 2.0f);

    // Plot the sine wave
    if (!values.empty()) {
      ImGui::PlotLines("Sine Wave", values.data(), values.size(), 0, nullptr,
                       -2.0f, 2.0f, ImVec2(0, 100));
    }
    ImGui::End();
  } catch (const std::exception& e) {
    fmt::print("ImGui rendering error: {}\n", e.what());
  }

  // Render ImGui
  ImGui::Render();
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

  SDL_RenderPresent(renderer);
}

#ifdef __EMSCRIPTEN__
void emscripten_loop(void* arg) {
  Application* app = static_cast<Application*>(arg);
  app->processEvents();
  app->update();
  app->render();
}
#endif

void Application::run() {
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(emscripten_loop, this, 0, true);
#else
  while (running) {
    processEvents();
    update();
    render();
  }
#endif
}

void Application::cleanup() {
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