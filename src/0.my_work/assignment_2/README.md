# Assignment 2: 3D Kinetic Sculpture Animation

## 🪐 Overview
This assignment implements a **3D Kinetic Sculpture Animation** using **OpenGL**.  
The project visualizes a dynamic galaxy-like structure composed of thousands of moving spheres, representing a kinetic sculpture in 3D space.  

The program demonstrates modern rendering techniques — including **instanced rendering**, **camera control**, and **real-time animation** — to efficiently display a large number of objects with unique positions and colors.  

The concept draws inspiration from kinetic art: sculptures that move through space and change form over time.  
In this case, the motion is simulated through rotation and perspective changes of a 3D spiral galaxy.

---

## ✨ Features

### 1. Procedural Galaxy Generation
- The function `generateGalaxy()` procedurally creates a set of star instances.
- Each instance has randomized:
  - **Position (x, y, z)** forming spiral arms.
  - **Color (r, g, b)** with subtle variance for realism.
- The galaxy structure simulates a rotating spiral formation with adjustable parameters:
  - `numStars`, `arms`, and `radius`.

### 2. Sphere Geometry Construction
- The function `generateSphere()` dynamically generates a 3D mesh for one sphere using latitude and longitude subdivision.
- Each sphere consists of:
  - Vertex positions (`x, y, z`).
  - Triangle indices for rendering smooth surfaces.
- This mesh acts as the **base geometry** for all star instances.

### 3. Instanced Rendering
- The program uses **`glDrawElementsInstanced()`** to render thousands of spheres efficiently in a single draw call.
- Instance attributes:
  - `instancePos` → controls each sphere’s world-space position.
  - `instanceColor` → defines individual star color.
- This drastically improves performance compared to drawing each sphere separately.

### 4. Real-time Animation
- The entire galaxy rotates slowly to simulate kinetic motion.
- The camera supports dynamic movement and zoom, allowing users to explore the structure from multiple perspectives.
- Frame updates are synchronized with delta time for smooth animation.

### 5. Camera and Lighting (optional extension)
- The `Camera` class implements FPS-style navigation using keyboard and mouse.
- Depth testing is enabled for proper 3D visualization.
- The scene background and motion lighting enhance the sense of depth and space.

---

## 🧩 Technical Summary

| Component | Description |
|------------|--------------|
| **Language** | C++ (Modern OpenGL / GLAD / GLFW) |
| **Rendering** | Instanced Mesh Rendering |
| **Geometry** | Procedurally generated sphere |
| **Instance Data** | Position + Color from galaxy generator |
| **Shader Model** | Vertex & Fragment shaders (GLSL 330) |
| **Camera** | 6-DOF navigation, perspective projection |
| **Performance** | >2000 instances rendered in real time |

---

## 🎮 Controls

| Key | Action |
|-----|---------|
| `W / A / S / D` | Move camera |
| Mouse | Look around |
| Scroll | Zoom in/out |
| `Esc` | Exit program |

---
## Build & Run

1. Create build directory and enter it:
```bash
mkdir -p build && cd build
```

2. Configure with CMake:
```bash
cmake ..
```

3. Build:
```bash
make
```

4. Return to repo root and go to the binary folder:
```bash
cd .. && cd bin/0.my_work
```

5. Run the executable:
```bash
./0.my_work__assignment_2
```