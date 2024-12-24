#include "Gui.hpp"

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

void Gui::ProcessEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT) {
      running = false;
    }
  }
}

void Gui::Update() {}

void Gui::Render() {
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
#ifndef __EMSCRIPTEN__
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
#endif
    // Retrieve the display size
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    float ymargin = 30.f;
    float xmargin = 10.f;
    float container_width = displaySize.x - xmargin;
    float container_height = displaySize.y - ymargin;
    // Static variables for resizable areas
    // 20% of the window width for left and right sidebar
    static float leftSidebarWidth = 0.2f * container_width;
    static float rightSidebarWidth = 0.2f * container_width;
    // 25% of the window height
    static float bottomBarHeight = 0.25f * container_height;

    // Constraints for minimum sizes
    const float minWidth = 100.0f;    // Minimum width for sidebars
    const float minHeight = 100.0f;   // Minimum height for the bottom bar
    const float splitterSize = 6.0f;  // Thickness of the splitter

    // Dynamically calculate remaining sizes
    float centerWidth = container_width - leftSidebarWidth - rightSidebarWidth -
                        2 * splitterSize;
    float mainAreaHeight =
        displaySize.y - bottomBarHeight - splitterSize - ymargin;

// Set up the fullscreen container
#ifdef __EMSCRIPTEN__
    ImGui::SetNextWindowPos(ImVec2(0, 0));
#else
    ImGui::SetNextWindowPos(ImVec2(0, ymargin));
#endif
    ImGui::SetNextWindowSize(displaySize);
    ImGuiWindowFlags containerFlags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("FullscreenContainer", nullptr, containerFlags);

    // Main Area (Top Section)
    ImGui::BeginChild("MainArea", ImVec2(container_width, mainAreaHeight),
                      false);

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
          container_width - rightSidebarWidth - centerWidth - splitterSize);
    }

    // Center Main Window
    ImGui::SameLine();
    ImGui::BeginChild("CenterMain", ImVec2(centerWidth, mainAreaHeight), true);
    ImGui::Text("Center Main Window");
    if (ImGui::Button(IsPaused() ? "Resume" : "Pause")) {
      TogglePause();
    }
    ImGui::SliderFloat("Frequency", &core_logic_.GetFrequency(), 0.1f, 10.0f);
    ImGui::SliderFloat("Amplitude", &core_logic_.GetAmplitude(), 0.1f, 10.0f);
    ImGui::SliderFloat("Frame per Seconds", &core_logic_.GetFps(), 5.f, 120.0f);

    // Plot the sine wave
    if (!core_logic_.GetSineWaveValues().empty()) {
      ImGui::PlotLines("Sine Wave", core_logic_.GetSineWaveValues().data(),
                       core_logic_.GetSineWaveValues().size(), 0, nullptr,
                       -10.0f, 10.0f, ImVec2(0, 300));
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
          container_width - leftSidebarWidth - centerWidth - splitterSize);
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
    ImGui::Button("BottomSplitter", ImVec2(container_width, splitterSize));
    ImGui::PopStyleColor(1);  // Pop the custom button styles
    if (ImGui::IsItemActive()) {
      bottomBarHeight -= io.MouseDelta.y;
      bottomBarHeight =
          ImClamp(bottomBarHeight, minHeight, displaySize.y * 0.5f);
    }

    // Bottom Bar
    ImGui::BeginChild("BottomBar", ImVec2(container_width, bottomBarHeight),
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