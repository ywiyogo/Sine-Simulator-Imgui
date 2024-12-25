# Sine Simulator with ImGui and SDL2

Simple example to create a desktop GUI and web app using C++, ImGui, and SDL2.

<video width="640" height="480" controls>
  <source src="docs/desktop.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

<video width="640" height="480" controls>
  <source src="docs/webapp.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

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
