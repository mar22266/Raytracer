# Ray Tracing Diorama

This project is a 3D ray tracing application that renders a dynamic diorama with various effects, including a skybox, different material properties, multiple light sources, and interactive camera controls. The user can move the camera using the keyboard to explore the scene, which is designed to showcase advanced ray tracing techniques.

## Demo

![Demo](demo/video.mp4)

This video demonstrates the main features and interactive controls of the Ray Tracing Diorama. Watch how the camera moves, the dynamic lighting changes, and the various material effects bring the scene to life.

## Features

- **Interactive Camera Control**: Move and rotate the camera using keyboard inputs to explore the scene.
- **Skybox**: An immersive background environment that surrounds the entire scene.
- **Performance**:The application runs at a frame rate between **10 and 25 FPS**, depending on the scene complexity and system performance. Optimizations may vary based on hardware settings.
- **Multiple Materials**: Various materials with different textures and properties like albedo, specularity, reflectivity, and transparency.
- **Dynamic Lighting**: Supports multiple light sources with different colors and intensities, including day-night cycles.
- **Normal Mapping**: Enhances surface details without increasing geometric complexity.
- **Emissive Materials**: Objects that act as light sources themselves.
- **Animated Textures**: Adds dynamic elements like flowing water or flickering fire.

## Controls

- **Rotation**: Use the `W`, `A`, `S`, `D` keys to move the camera forward, backward, left, and right, respectively.
- **Zoom In/Out**: Use the `Up` and `Down` arrow keys to zoom the camera in and out.

## Project Structure

- `main.cpp`: Entry point of the program that sets up the scene and manages the rendering loop.
- `camera.h` / `camera.cpp`: Defines the camera and its controls.
- `cube.h` / `cube.cpp`: Manages cube objects and their material properties.
- `material.h`: Defines various material types with specific textures and effects.
- `skybox.h` / `skybox.cpp`: Manages the skybox rendering around the scene.
- `light.h`: Defines light sources and their properties.
- `color.h` / `color.cpp`: Handles color manipulation within the scene.
- `intersect.h`: Contains functions for calculating intersections between rays and objects.
- `object.h`: Provides a base structure for all objects in the scene.

## Installation and Setup

### Prerequisites

- CMake 3.10 or higher
- A C++17 compatible compiler
- SDL2 and SDL_image libraries
- GLM library
- TBB (Threading Building Blocks) for parallel computing

### Build and Run Instructions

1. **Configure the Project**:
   Run the script to set up the CMake build files.

   ```bash
   ./configure.sh
   ./build.sh
   ./run.sh
   ./clean.sh

### Explanation of Files and How to Run

- **`configure.sh`**: Sets up the CMake build configuration in the `build` directory.
- **`build.sh`**: Compiles the project by navigating to the `build` directory and running `make`.
- **`run.sh`**: Combines the configuration and build steps, then runs the compiled application.
- **`clean.sh`**: Cleans up the build directory by removing all generated files.

This README should provide a comprehensive guide for users to understand, set up, and run your project. Let me know if you need further adjustments or additional details!


