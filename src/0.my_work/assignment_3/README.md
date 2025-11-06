# Assignment 3: Simple 3D Game

## 🎮 Overview

This project implements a **Simple 3D Game** using **OpenGL**.
The game demonstrates 3D model loading, player control, camera following, and basic collision detection.
All models (player, scene, items, enemies) are loaded from external files, showcasing animation and interaction in a small 3D world.

The player can freely move around the environment, and the camera dynamically follows the player’s position.
This project serves as a foundation for understanding how game engines handle real-time rendering, input control, and scene management.

---

## ✨ Features

### 1. 3D Model Loading

- Uses **Assimp** to load 3D models in `.dae` (Collada) or `.obj` format.
- The player’s model is imported from:

  ```
  resources/objects/Longbow Locomotion Pack/
  ├─ Ch39_nonPBR.dae                (main mesh)
  ├─ Standing Idle 01.dae           (idle animation)
  └─ Standing Run Forward.dae       (running animation)
  ```

- Supports loading additional scene objects such as floors, walls, and collectible items.

---

### 2. Player Control

- The player model can move using keyboard input:

  - `W / A / S / D` for movement
  - `Shift` for running

- Player animation switches dynamically based on movement:

  - **Idle** when stationary
  - **Run** when moving

---

### 3. Camera System

- Implements a **third-person camera** that smoothly follows the player’s position.
- The camera updates its position every frame:

  ```cpp
  cameraPos = player.position - cameraFront * 5.0f + glm::vec3(0, 2, 0);
  view = glm::lookAt(cameraPos, player.position, glm::vec3(0, 1, 0));
  ```

- Mouse movement controls the viewing direction around the player.

---

### 4. Collision Detection

- Simple collision system using **Axis-Aligned Bounding Boxes (AABB)** or **bounding spheres**.
- Detects collisions between:

  - Player ↔ Scene (walls, ground)
  - Player ↔ Items (collectible interactions)

- Example function:

  ```cpp
  bool isColliding(const AABB& a, const AABB& b) {
      return (a.max.x > b.min.x && a.min.x < b.max.x) &&
             (a.max.y > b.min.y && a.min.y < b.max.y) &&
             (a.max.z > b.min.z && a.min.z < b.max.z);
  }
  ```

---

### 5. Scene Environment

- A basic scene includes:

  - Ground plane
  - Static obstacles or walls
  - Items that can be collected

- All objects are imported from model files for modularity.

---

## 🧩 Technical Summary

| Component        | Description                                      |
| ---------------- | ------------------------------------------------ |
| **Language**     | C++ (Modern OpenGL / GLAD / GLFW / Assimp)       |
| **Models**       | Player, scene, and item models loaded from files |
| **Animation**    | Skeletal animation via imported `.dae` files     |
| **Camera**       | Third-person following camera                    |
| **Collision**    | Basic AABB or bounding sphere detection          |
| **Input System** | Keyboard + Mouse control                         |
| **Rendering**    | Depth-tested 3D rendering with lighting          |

---

## 🎮 Controls

| Key             | Action        |
| --------------- | ------------- |
| `W / A / S / D` | Move player   |
| `Shift`         | Run           |
| Mouse           | Rotate camera |
| Scroll          | Zoom in/out   |
| `Esc`           | Exit program  |

---

## 🏗️ Build & Run

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

4. Go to the binary output folder:

```bash
cd .. && cd bin/0.my_work
```

5. Run the executable:

```bash
./0.my_work__assignment_3
```

---
