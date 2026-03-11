# Vampire Engine

A lightweight Vulkan-based 3D rendering engine prototype featuring model loading via Assimp, custom shader integration, and efficient buffer management.

## Project Architecture

The engine is structured into several core components:
- **Context:** Manages Vulkan instance, physical/logical devices, and surface creation.
- **Renderer:** Handles the swapchain, render passes, framebuffers, and the graphics pipeline.
- **Graphic:** Contains high-level functions for model loading, texture creation, and command buffer recording.
- **Platform:** Manages window creation and OS-specific attributes (e.g., Windows Dark Mode).

## Core Features

### 1. Vertex Structure & Shaders
The engine uses a standardized `Vertex` structure for all 3D models:
- `aPos` (location 0): `vec3` - Vertex position.
- `aNormal` (location 1): `vec3` - Vertex normal for lighting calculations.
- `aUV` (location 2): `vec2` - Texture coordinates.

**Shaders:**
- `shader.vert`: Transforms vertices using MVP matrices and passes normals and UVs to the fragment shader.
- `shader.frag`: Performs basic diffuse lighting and texture sampling using `sampler2D`.

### 2. Multi-Mesh Model Loading
The `load_model` function uses the **Assimp** library to import 3D files (FBX, OBJ, etc.). It supports:
- **Mesh Separation:** Each mesh in a model file is stored as a separate `Mesh` object.
- **Automatic Buffer Creation:** For every mesh, the engine automatically creates and populates dedicated Vulkan vertex and index buffers.
- **Normals & UVs:** Automatically generates smooth normals if missing and handles multiple UV sets.

### 3. Buffer Management
The engine employs a `Mesh` struct to encapsulate vertex and index data:
```cpp
typedef struct {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    uint32_t indexCount;
    uint32_t vertexCount;
} Mesh;
```
- **Staging Buffers:** Uses high-speed transfer operations to move data from CPU-visible memory to GPU-local memory for optimal performance.
- **Dynamic Drawing:** The rendering loop iterates through `state->renderer.meshes`, binding and drawing each one individually within the command buffer.

### 4. Texture & Uniform Support
- **Textures:** Supports JPG/PNG textures via `stb_image`. Currently uses a global texture sampler.
- **Uniforms:** A `UniformBufferObject` (UBO) provides Model, View, and Projection matrices to the vertex shader, updated every frame.

## How It Works

1.  **Initialization:** The `init` function sets up Vulkan, creates the swapchain, and initializes the `commandPool`.
2.  **Resource Loading:** Textures and models are loaded. `load_model` populates the `meshes` vector.
3.  **Pipeline Creation:** The graphics pipeline is built using SPIR-V shaders, defining the vertex input state to match our `Vertex` struct.
4.  **Frame Loop:**
    *   `draw_frame` waits for the GPU, acquires an image from the swapchain, and updates the UBO.
    *   `record_command_buffer` is called, which loops through all loaded meshes and issues `vkCmdDrawIndexed` calls.
    *   The command buffer is submitted to the graphics queue for presentation.

## Building the Project

### Prerequisites
- **Vulkan SDK** (Environment variable `VULKAN_SDK` must be set).
- **CMake** 3.20 or higher.
- **C++20** compatible compiler (e.g., GCC 11+, Clang 12+, MSVC 2019+).
- **glslc** (included with Vulkan SDK) for shader compilation.

### Build Steps
1.  **Compile Shaders:**
    ```bash
    glslc shaders/shader.vert -o shaders/shader.vert.spv
    glslc shaders/shader.frag -o shaders/shader.frag.spv
    ```
2.  **Configure & Build:**
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```
3.  **Run:**
    Execute `app.exe` (or `./app` on Linux) from the root directory or ensure the `assets` and `shaders` folders are accessible relative to the executable.
