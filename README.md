# 🎮 3D Desert Eagle Pistol Model

A colorful 3D graphics project that models a Desert Eagle-style pistol and demonstrates interactive transformations. The model features a chrome-colored slide and barrel, steel frame, red grip, and brass-colored details.

## 👥 Authors

- **Shashwat Kushwaha**
- **Soumyajit Debnath**

## ✨ Features

- 3D pistol model with colored components
- Interactive translation, scaling, rotation, reflection, and shearing
- Solid and wireframe display modes
- Automatic demonstration mode for transformations
- Perspective projection, flat shading, and page flipping for smoother rendering

## 🎮 Controls

| Keys | Action |
| --- | --- |
| `A` / `D` | Move along X |
| `W` / `S` | Move along Y |
| `Q` / `E` | Move along Z |
| `+` / `-` | Scale uniformly up / down |
| `N` / `M` | Scale along X down / up |
| `1` / `2` | Rotate around X axis |
| `3` / `4` | Rotate around Y axis |
| `5` / `6` | Rotate around Z axis |
| `X` / `Y` / `Z` | Reflect across the corresponding plane |
| `H` / `J` | Shear X by Y |
| `K` / `L` | Shear Y by X |
| `F` | Toggle solid / wireframe view |
| `O` | Toggle automatic demo |
| `R` | Reset the model |
| `Esc` | Exit |

## 🛠️ Requirements

The source uses the classic `graphics.h` BGI graphics library and Turbo C++-style graphics functions. A compatible Turbo C++/BGI environment is required to build and run it. The program currently initializes the BGI graphics driver from `C:\TC\BGI`; update that path in `deagle.cpp` if your BGI files are installed elsewhere.

## ▶️ Run

1. Open `deagle.cpp` in a Turbo C++ environment configured with BGI.
2. Confirm the BGI path in the `initgraph()` call.
3. Compile and run the program, then use the controls above to explore the model and transformations.

---

Made with ❤️ by **Shashwat Kushwaha** and **Soumyajit Debnath**.