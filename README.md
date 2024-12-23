# Sine Simulator with ImGui and SDL2

Simple example to create a desktop GUI and web app using C++, ImGui, and SDL2.

## Getting Started on Arch Linux

Install basic development tools:
```
sudo pacman -S base-devel cmake sdl2 emscripten
```

## Build and run the application

For native desktop build :

```bash
chmod +x scripts/build_native.sh
./scripts/build_native.sh
./build/native/sine-simulator
```

For web build :

```bash
chmod +x scripts/build_web.sh
./scripts/build_web.sh
cd build/web
python -m http.server 8080
```
