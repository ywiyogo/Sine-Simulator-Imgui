#include "CoreLogic.hpp"
#include "Gui.hpp"
#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/html5.h>

struct EmscriptenLoopArgs {
  Gui* gui;
  CoreLogic* coreLogic;
};

void emscripten_loop(void* arg) {
  EmscriptenLoopArgs* args = static_cast<EmscriptenLoopArgs*>(arg);
  Gui* gui = args->gui;
  CoreLogic* coreLogic = args->coreLogic;

  gui->ProcessEvents();

  if (!gui->IsPaused()) {
    coreLogic->Update();
  }
  gui->Run();
}
#endif

int main(int argc, char* argv[]) {

  CoreLogic coreLogic;
  Gui gui(coreLogic);

  if (!gui.Initialize()) {
    return 1;
  }
#ifdef __EMSCRIPTEN__
  EmscriptenLoopArgs loopArgs = {&gui, &coreLogic};
  emscripten_set_main_loop_arg(emscripten_loop, &loopArgs, 0, true);
#else
  while (gui.IsRunning()) {
    if (!gui.IsPaused()) {
      coreLogic.Update();
    }
    gui.Run();
  }
#endif
  return 0;
}
