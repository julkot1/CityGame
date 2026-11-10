# CityBuilder

A lightweight C++ city builder prototype built with [Magnum Graphics Engine](https://magnum.graphics/), [Corrade](https://corrade.graphics/), and SDL2.


---

## Prerequisites

Ensure you have the following installed on your system:

- **C++ Compiler** with C++17 support (`g++` or `clang++`)
- **CMake** (version 3.10 or newer)
- **SDL2** development libraries
  - Ubuntu/Debian: `sudo apt install libsdl2-dev`
  - Arch Linux: `sudo pacman -S sdl2`
  - Fedora: `sudo dnf install SDL2-devel`
- **OpenGL** libraries

---

## Getting Started

### 1. Clone the Repository

Clone with submodules included:

```bash
git clone --recursive <repository-url>
cd CityBuilder
```

If you already cloned without `--recursive`, initialize the submodules:

```bash
git submodule update --init --recursive
```

### 2. Build the Project

```bash
# Configure
cmake -B build

# Build
make -C build
```

### 3. Run

```bash
./build/bin/app
```
