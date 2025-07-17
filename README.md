# Cherry Engine

![Cherry Engine Banner](https://user-images.githubusercontent.com/8225057/191050833-b7ecf528-bfae-4a9f-ac1b-f3d83437a2f4.png)

Cherry Engine is a modular, modern C++17 game engine/framework for Windows, designed for real-time rendering, extensibility, and rapid development. It leverages OpenGL for graphics, ImGui for UI, and a robust event/layer system for clean architecture.

---

## 🚀 Features

- **Layered Architecture:** Modular system for rendering, UI, and logic layers.
- **Immediate Event System:** Type-safe, immediate event dispatching for input and window events.
- **ImGui Integration:** Built-in support for ImGui overlays and tools.
- **OpenGL Rendering:** Modern OpenGL pipeline with vertex/index buffers and shaders.
- **Cross-Platform Dependencies:** Uses GLFW, Glad, ImGui, glm, and spdlog.
- **Sandbox Example:** Sample application to help you get started quickly.

---

## 🖼️ Visuals

Cherry Engine is designed for real-time graphics and UI overlays. By default, it renders a colored triangle and an ImGui overlay:

### Example ImGui Window

![ImGui Example](https://user-images.githubusercontent.com/8225057/191050833-b7ecf528-bfae-4a9f-ac1b-f3d83437a2f4.png)

### Example OpenGL Output (Default Triangle)

```
+----------------------+
|        ▲             |
|       ▲▲▲            |
|      ▲▲▲▲▲           |
|     ▲▲▲▲▲▲▲          |
|    ▲▲▲▲▲▲▲▲▲         |
|   ▲▲▲▲▲▲▲▲▲▲▲        |
|  ▲▲▲▲▲▲▲▲▲▲▲▲▲       |
+----------------------+
```

---

## 🏗️ Getting Started

### Prerequisites
- Windows (x64)
- Visual Studio 2019+ or compatible C++17 compiler
- [Premake5](https://premake.github.io/) for project generation

### Build Instructions

1. **Clone the repository (with submodules):**
   ```sh
   git clone --recursive <repo-url>
   ```
2. **Generate project files:**
   ```sh
   ./GenerateProject.bat
   ```
3. **Open the generated solution in Visual Studio and build.**

### Running the Example
- The `Sandbox` project demonstrates how to create a custom application using Cherry Engine.
- Press `ESC` in the running window to trigger an exit confirmation dialog.

---

## 🗂️ Project Structure

```plaintext
Cherry/
  ├─ src/Cherry/         # Engine source code
  ├─ Vendor/             # Third-party libraries (GLFW, Glad, ImGui, glm, spdlog)
  ├─ Sandbox/            # Example application
  ├─ premake5.lua        # Build configuration
  └─ README.md           # This file
```

---

## 🧩 Extending Cherry Engine

- **Create a new Layer:** Inherit from `Cherry::Layer` and override `OnUpdate`, `OnImGuiRender`, and `OnEvent`.
- **Add your Layer:** Use `PushLayer(new MyLayer())` in your custom `Application` subclass.
- **Use ImGui:** Draw custom UI in `OnImGuiRender()`.

---

## 📦 Dependencies

- [GLFW](https://www.glfw.org/) - Windowing and input
- [Glad](https://github.com/Dav1dde/glad) - OpenGL loader
- [ImGui](https://github.com/ocornut/imgui) - Immediate-mode GUI
- [glm](https://github.com/g-truc/glm) - Math library
- [spdlog](https://github.com/gabime/spdlog) - Logging

---

## 🤝 Contributing

Contributions are welcome! Please open issues or pull requests for bugs, features, or documentation improvements.

---

## 📄 License

Cherry Engine is released under the MIT License.

---

> **Cherry Engine is under active development. Stay tuned for more features and improvements!**
