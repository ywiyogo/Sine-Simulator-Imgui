#include "Application.hpp"

#include <cmath>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
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
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fmt::print("SDL initialization failed: {}", SDL_GetError());
    return false;
  }

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

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  if (!ImGui_ImplSDL2_InitForSDLRenderer(window, renderer)) {
    fmt::print("ImGui SDL2 initialization failed\n");
    return false;
  }

  if (!ImGui_ImplSDLRenderer2_Init(renderer)) {
    fmt::print("ImGui SDL2 Renderer initialization failed\n");
    return false;
  }

  LoadSystemFonts();
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
    // Create main menu bar
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New", "Ctrl+N")) {
        }
        if (ImGui::MenuItem("Open", "Ctrl+O")) {
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "Alt+F4")) {
          running = false;
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
        }
        if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Left Sidebar", nullptr, &showLeftSidebar);
        ImGui::MenuItem("Right Sidebar", nullptr, &showRightSidebar);
        ImGui::MenuItem("Bottom Panel", nullptr, &showBottomPanel);
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }
    const float y_offset = FONT_SIZE + 5.f;
    // Left Sidebar
    ImGui::SetNextWindowPos(ImVec2(0, y_offset));
    ImGui::SetNextWindowSize(
        ImVec2(200, ImGui::GetIO().DisplaySize.y - y_offset));

    if (ImGui::Begin("Left Sidebar", nullptr)) {
      ImGui::Text("Sidebar content");
      ImGui::End();
    }
    // Right Sidebar
    ImGui::SetNextWindowPos(
        ImVec2(ImGui::GetIO().DisplaySize.x - 200, y_offset));
    ImGui::SetNextWindowSize(
        ImVec2(200, ImGui::GetIO().DisplaySize.y - y_offset));
    if (ImGui::Begin("Right Sidebar", nullptr)) {
      ImGui::Text("Sidebar content");
      ImGui::End();
    }

    // Main Center Window
    ImGui::SetNextWindowPos(ImVec2(200, y_offset));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - 400,
                                    ImGui::GetIO().DisplaySize.y - 60));
    if (ImGui::Begin("Main Center Window", nullptr)) {
      ImGui::Text("Main content goes here");
      ImGui::SliderFloat("Frequency", &frequency, 0.1f, 5.0f);
      ImGui::SliderFloat("Amplitude", &amplitude, 0.1f, 2.0f);

      // Plot the sine wave
      if (!values.empty()) {
        ImGui::PlotLines("Sine Wave", values.data(), values.size(), 0, nullptr,
                         -2.0f, 2.0f, ImVec2(0, 300));
      }
      ImGui::End();
    }

    // Bottom Window
    ImGui::SetNextWindowPos(ImVec2(200, ImGui::GetIO().DisplaySize.y - 200));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - 400, 200));
    if (ImGui::Begin("Bottom Window", nullptr, ImGuiWindowFlags_NoDocking)) {
      ImGui::Text("Bottom content");
      ImGui::End();
    }

    // Create ImGui window
    ImGui::Begin("New window");

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