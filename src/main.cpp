#include "Application.hpp"

int main(int argc, char* argv[]) {
  Application app;

  if (!app.initialize()) {
    return 1;
  }

  app.run();
  return 0;
}
