# 🧊 OpenGL 3D Rubik's Cube

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![OpenGL](https://img.shields.io/badge/OpenGL-%23FFFFFF.svg?style=for-the-badge&logo=opengl)
![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)
![License](https://img.shields.io/badge/license-MIT-green?style=for-the-badge)

> **A fully interactive, procedurally generated 3D Rubik's Cube engine built from scratch in C.**

---

## 🖼️ Preview

<!-- UBACI SCREENSHOT OVDE KASNIJE: -->
<!-- ![Gameplay Screenshot](path/to/image.png) -->
*(Place a cool screenshot or GIF of your cube here to grab attention!)*

---

## 🚀 About The Project

This isn't just a static model. It's a complete **3D simulation engine** written in pure C using modern OpenGL (Core Profile). It features a custom-built hierarchy system to handle the complex rotations of the cube layers, complete with smooth animations and sound effects.

### ✨ Key Features

*   **Procedural Geometry:** The cube is generated via code, not loaded from a model file.
*   **Hierarchical Animations:** Smooth, interpolated layer rotations using matrix transformations.
*   **Skybox Environment:** Immersive 3D background using Cubemaps.
*   **Audio System:** Integrated `miniaudio` for satisfying mechanical sound effects.
*   **Auto-Solve Logic:** A stack-based history system that can reverse time and solve the cube automatically.
*   **Modern OpenGL:** Uses Shaders (GLSL 3.30), VAOs, and VBOs.

---

## 🎮 Controls

| Key | Action |
| :---: | :--- |
| **Mouse** | Hold **Left Click** to rotate the camera (Orbit) |
| **I / K** | Rotate **Vertical** Layer (Up / Down) |
| **J / L** | Rotate **Horizontal** Layer (Left / Right) |
| **U / O** | Rotate **Depth** Layer (Front / Back) |
| **S** | **Shuffle** (Randomize the cube) |
| **SPACE** | **Auto-Solve** (Watch it solve itself) |
| **H** | Show Help in Console |
| **ESC** | Exit |

---

## 🛠️ Tech Stack

*   **Language:** C (C99 Standard)
*   **Graphics:** OpenGL 3.3 Core
*   **Windowing:** GLFW
*   **Math:** cglm (Optimized OpenGL Math)
*   **Audio:** miniaudio
*   **Build System:** CMake

---

## 📦 How to Build & Run

### Prerequisites
Ensure you have a C compiler (GCC/Clang/MSVC) and CMake installed.

### Build Steps

```bash
# 1. Clone the repository
git clone https://github.com/sshegy/OpenGL_3D.git
cd OpenGL_3D

# 2. Create build directory
mkdir build
cd build

# 3. Generate build files with CMake
cmake ..

# 4. Compile
make

# 5. Run
./OpenGL_3D
