# Cherry Engine - Code Review Report

## 🔴 Critical Bugs

### 1. **OpenGL Index Buffer Bug** (CRITICAL)
**File:** `Cherry/src/Platform/OpenGL/OpenGLBuffer.cpp:54`
**Issue:** Index buffer is created with `GL_ARRAY_BUFFER` instead of `GL_ELEMENT_ARRAY_BUFFER`
```cpp
// Current (WRONG):
glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);

// Should be:
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
```
**Impact:** Index buffers may not work correctly, causing rendering failures.

---

### 2. **Memory Leak - Renderer::m_SceneData** (CRITICAL)
**File:** `Cherry/src/Cherry/Renderer/Renderer.cpp:10`
**Issue:** Memory allocated with `new` but never deleted
```cpp
Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;
```
**Fix:** Add `Renderer::Shutdown()` method and delete `m_SceneData`, or use smart pointer.

---

### 3. **Memory Leak - Application::m_ImGuiLayer** (CRITICAL)
**File:** `Cherry/src/Cherry/Core/Application.cpp:22`
**Issue:** `ImGuiLayer` allocated with `new` but never deleted
```cpp
m_ImGuiLayer = new ImGuiLayer();
```
**Fix:** Use smart pointer or delete in destructor.

---

### 4. **Memory Leak - WindowsWindow::m_Context** (CRITICAL)
**File:** `Cherry/src/Platform/Windows/WindowsWindow.cpp:61`
**Issue:** `OpenGLContext` allocated with `new` but never deleted
```cpp
m_Context = new OpenGLContext(m_Window);
```
**Fix:** Use smart pointer or delete in `Shutdown()` method.

---

### 5. **Memory Leak - Renderer2D::s_Data** (CRITICAL)
**File:** `Cherry/src/Cherry/Renderer/Renderer2D.cpp:27`
**Issue:** Memory allocated but `Shutdown()` is never called
**Fix:** Call `Renderer2D::Shutdown()` in `Application` destructor or `Renderer::Shutdown()`.

---

## 🟡 Bugs & Issues

### 6. **File Reading Bug**
**File:** `Cherry/src/Platform/OpenGL/OpenGLShader.cpp:66`
**Issue:** Comparison `size != -1` is incorrect - `tellg()` returns `std::streampos` which when cast to `size_t` can't be -1
```cpp
// Current:
size_t size = in.tellg();
if (size != -1)  // This will never be true if size_t is unsigned

// Should be:
in.seekg(0, std::ios::end);
std::streampos pos = in.tellg();
if (pos != std::streampos(-1)) {
    size_t size = static_cast<size_t>(pos);
    // ...
}
```

---

### 7. **Missing Uniform Location Validation**
**File:** `Cherry/src/Platform/OpenGL/OpenGLShader.cpp:231-270`
**Issue:** `glGetUniformLocation` can return -1 (invalid location), but code doesn't check before using
**Impact:** Silent failures when setting uniforms that don't exist in shader
**Fix:** Add validation:
```cpp
GLint location = glGetUniformLocation(m_RendererID, name.c_str());
if (location == -1) {
    CH_CORE_WARN("Uniform '{0}' not found in shader", name);
    return;
}
glUniform1i(location, value);
```

---

### 8. **Include Typo**
**File:** `Cherry/src/Platform/OpenGL/OpenGLVertexArray.cpp:1`
**Issue:** `#include "Chpch.h"` should be `#include "CHpch.h"` (case sensitivity on some systems)

---

### 9. **Comma Operator Instead of Assignment**
**File:** `Cherry/src/Platform/OpenGL/OpenGLTexture.cpp:14`
**Issue:** Using comma operator instead of proper assignment
```cpp
// Current:
m_InternalFormat = GL_RGBA8, m_DataFormat = GL_RGBA;

// Should be:
m_InternalFormat = GL_RGBA8;
m_DataFormat = GL_RGBA;
```

---

### 10. **Potential Issue with Texture Size Validation**
**File:** `Cherry/src/Platform/OpenGL/OpenGLTexture.cpp:88`
**Issue:** Size calculation assumes 3 or 4 channels, but doesn't handle other formats
**Note:** This is okay if you only support RGB/RGBA, but consider making it more robust.

---

## 🟢 Code Quality & Improvements

### 11. **LayerStack Memory Management**
**File:** `Cherry/src/Cherry/Core/LayerStack.cpp:12-13`
**Issue:** `LayerStack` deletes layers in destructor, but ownership is unclear
**Recommendation:** Document ownership clearly or use smart pointers.

---

### 12. **Missing Error Handling**
- `OpenGLShader::ReadFile()` doesn't handle file read failures gracefully
- Texture loading doesn't check if `stbi_load` returns nullptr before assertion
- No validation that shader compilation succeeded before linking

---

### 13. **Hard-coded Values**
- `GL_STATIC_DRAW` is hard-coded in buffer creation (noted in TODO comments)
- Consider making buffer usage configurable

---

### 14. **GLFW Cleanup**
**File:** `Cherry/src/Platform/Windows/WindowsWindow.cpp:48`
**Issue:** TODO comment says "glfwTerminate on system shutdown" but it's never implemented
**Fix:** Add cleanup in Application destructor or use RAII wrapper.

---

### 15. **Missing Move Semantics**
Consider adding move constructors/assignment operators for better performance with large objects.

---

### 16. **Shader Type Validation**
**File:** `Cherry/src/Platform/OpenGL/OpenGLShader.cpp:101`
**Issue:** Assert checks if `ShaderTypeFromString` returns non-zero, but should check for specific valid types

---

### 17. **Renderer2D Shutdown Not Called**
**File:** `Cherry/src/Cherry/Renderer/Renderer2D.h:11`
**Issue:** `Shutdown()` method exists but is never called
**Fix:** Call in `Application` destructor or `Renderer::Shutdown()`

---

## 📊 Summary

- **Critical Bugs:** 5 (Memory leaks + Index buffer bug)
- **Bugs:** 5 (File reading, validation, typos)
- **Improvements:** 7 (Code quality, error handling, cleanup)

## 🎯 Priority Fixes

1. **Fix Index Buffer Bug** - Will cause rendering failures
2. **Fix Memory Leaks** - Will cause resource exhaustion over time
3. **Add Uniform Validation** - Will help debug shader issues
4. **Fix File Reading** - May cause crashes on file errors
5. **Add Shutdown Calls** - Proper cleanup on exit

