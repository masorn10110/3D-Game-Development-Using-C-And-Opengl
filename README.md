# SPECIAL TOPIC IV: 3D GAME DEVELOPMENT USING C AND OPENGL

---

## 🌀 Overview

This project is part of the **SPECIAL TOPIC IV: 3D Game Development Using C and OpenGL** course.  
The course introduces 3D computer graphics through the **OpenGL graphics library**, focusing on mathematical foundations, 3D rendering algorithms, lighting, shading, and animation systems.

This particular assignment — **3D Kinetic Sculpture Animation** — applies those principles by generating a dynamic, abstract 3D sculpture made from procedural geometry.  
It demonstrates:

- Procedural generation of **sphere meshes** and **galactic formations**
- Smooth **keyframe-like motion** and **rotational animation**
- Basic **lighting and shading** in GLSL shaders
- **Camera control** for interactive observation

---

## 🧱 Project Structure

```markdown
📁 LearnOpenGL/
├── 📁 configuration/ # Configuration files for environment or build
├── 📁 dlls/ # Precompiled dynamic libraries (Windows)
├── 📁 includes/ # Header includes for external dependencies
├── 📁 lib/ # Static and shared libraries
├── 📁 resources/ # Assets used by projects
│ ├── audio/
│ ├── fonts/
│ ├── levels/
│ ├── objects/
│ └── textures/
├── 📁 src/
│ ├── 📁 0.my_work/
│ │ └── 📁 assignment_2/
│ │ │ ├── images
│ │ │ │ ├── assignment_2_1.png
│ │ │ │ └── ...
│ │ │ ├── videos
│ │ │ │ └── assignment_2.mov
│ │ │ ├── assignment_2.cpp # Main source code
│ │ │ ├── assignment_2.vs # Vertex shader
│ │ │ └── assignment_2.fs # Fragment shader
│ │ └── README.md
│ ├── glad.c
│ └── stb_image.cpp
├── build.sh # Build script for Linux/macOS
├── build_windows.sh # Build script for Windows
├── CMakeLists.txt # CMake build configuration
├── LICENSE.md
└── README.md
```

---

## ⚡ Quick Start

### Prerequisites

Make sure you have these installed:

- CMake ≥ 3.10
- C++17 compatible compiler
- OpenGL 3.3+
- Libraries: `GLFW`, `GLAD`, `GLM`, `ASSIMP`, `FREETYPE`

### 🖥️ Build Instructions

#### Step 1. Clone the repository

```bash
git clone https://github.com/yourusername/Assignment2-KineticSculpture.git

cd Assignment2-KineticSculpture
```

#### Step 2. Configure the project with CMake

```bash
mkdir -p build && cd build
cmake ..
```

#### Step 3. Build the executable

```bash
make
```

#### Step 4. Run the program

On Windows:

```bash
cd ..
./bin/0.my_work/${Filename}.exe
```

On Linux/macOS:

```bash
cd ..
./bin/0.my_work/${Filename}
```

**Note**: if u need to know what file can run after bulid it will show list or can see in ./src/0.my_work/worklist.txt
