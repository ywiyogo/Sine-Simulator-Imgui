#include "Application.hpp"

#include <cmath>

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
  ImGui::SetupImGuiStyle(false);
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

    // Retrieve the display size
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    float ymargin = 30.f;
    // Static variables for resizable areas
    static float leftSidebarWidth =
        0.2f * displaySize.x;  // 20% of the window width
    static float rightSidebarWidth =
        0.2f * displaySize.x;  // 20% of the window width
    static float bottomBarHeight =
        0.25f * displaySize.y;  // 20% of the window height

    // Constraints for minimum sizes
    const float minWidth = 100.0f;    // Minimum width for sidebars
    const float minHeight = 100.0f;   // Minimum height for the bottom bar
    const float splitterSize = 6.0f;  // Thickness of the splitter

    // Dynamically calculate remaining sizes
    float centerWidth =
        displaySize.x - leftSidebarWidth - rightSidebarWidth - 2 * splitterSize;
    float mainAreaHeight =
        displaySize.y - bottomBarHeight - splitterSize - ymargin;

    // Set up the fullscreen container
    ImGui::SetNextWindowPos(ImVec2(0, ymargin));
    ImGui::SetNextWindowSize(displaySize);
    ImGuiWindowFlags containerFlags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("FullscreenContainer", nullptr, containerFlags);

    // Main Area (Top Section)
    ImGui::BeginChild("MainArea", ImVec2(displaySize.x, mainAreaHeight), false);

    // Left Sidebar
    ImGui::BeginChild("LeftSidebar", ImVec2(leftSidebarWidth, mainAreaHeight),
                      true);
    ImGui::Text("Left Sidebar");
    ImGui::Button("Option 1");
    ImGui::Button("Option 2");
    ImGui::EndChild();

    // Splitter for Left Sidebar and Center
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button,
                          IM_COL32(190, 190, 190, 255));  // Darker color
    ImGui::Button("LeftSplitter", ImVec2(splitterSize, mainAreaHeight));
    ImGui::PopStyleColor(1);  // Pop the custom button styles
    if (ImGui::IsItemActive()) {
      leftSidebarWidth += io.MouseDelta.x;
      leftSidebarWidth = ImClamp(
          leftSidebarWidth, minWidth,
          displaySize.x - rightSidebarWidth - centerWidth - splitterSize);
    }

    // Center Main Window
    ImGui::SameLine();
    ImGui::BeginChild("CenterMain", ImVec2(centerWidth, mainAreaHeight), true);
    ImGui::Text("Center Main Window");
    ImGui::Button("Main Button");
    ImGui::SliderFloat("Frequency", &frequency, 0.1f, 5.0f);
    ImGui::SliderFloat("Amplitude", &amplitude, 0.1f, 2.0f);

    // Plot the sine wave
    if (!values.empty()) {
      ImGui::PlotLines("Sine Wave", values.data(), values.size(), 0, nullptr,
                       -2.0f, 2.0f, ImVec2(0, 300));
    }
    ImGui::EndChild();

    // Splitter for Center and Right Sidebar
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button,
                          IM_COL32(190, 190, 190, 255));  // Darker color
    ImGui::Button("RightSplitter", ImVec2(splitterSize, mainAreaHeight));
    ImGui::PopStyleColor(1);  // Pop the custom button styles
    if (ImGui::IsItemActive()) {
      rightSidebarWidth -= io.MouseDelta.x;
      rightSidebarWidth = ImClamp(
          rightSidebarWidth, minWidth,
          displaySize.x - leftSidebarWidth - centerWidth - splitterSize);
    }

    // Right Sidebar
    ImGui::SameLine();
    ImGui::BeginChild("RightSidebar", ImVec2(rightSidebarWidth, mainAreaHeight),
                      true);
    ImGui::Text("Right Sidebar");
    ImGui::Button("Setting 1");
    ImGui::Button("Setting 2");
    ImGui::EndChild();

    ImGui::EndChild();  // End of MainArea

    // Splitter for Main Area and Bottom Bar

    ImGui::PushStyleColor(ImGuiCol_Button,
                          IM_COL32(190, 190, 190, 255));  // Darker color
    ImGui::Button("BottomSplitter", ImVec2(displaySize.x, splitterSize));
    ImGui::PopStyleColor(1);  // Pop the custom button styles
    if (ImGui::IsItemActive()) {
      bottomBarHeight -= io.MouseDelta.y;
      bottomBarHeight =
          ImClamp(bottomBarHeight, minHeight, displaySize.y * 0.5f);
    }

    // Bottom Bar
    ImGui::BeginChild("BottomBar", ImVec2(displaySize.x, bottomBarHeight),
                      true);
    ImGui::Text("Bottom Bar");
    ImGui::Button("Action 1");
    ImGui::Button("Action 2");
    ImGui::EndChild();

    ImGui::End();  // End of FullscreenContainer

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