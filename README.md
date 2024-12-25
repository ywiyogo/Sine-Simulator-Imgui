# Sine Simulator with ImGui and SDL2

Simple example to create a desktop GUI and web app using C++, ImGui, and SDL2.

https://github.com/ywiyogo/Sine-Simulator-Imgui/tree/master/docs/desktop.mp4

![Web app Preview](docs/webapp.mp4)
https://github.com/ywiyogo/Sine-Simulator-Imgui/tree/master/docs/webapp.mp4

## Getting Started 

### Arch Linux

Install basic development tools:
```
sudo pacman -S base-devel cmake sdl2 emscripten
```

### Ubuntu

```
sudo apt install -y build-essential cmake libsdl2-dev emscripten
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
./scripts/run_webserver.sh
```
