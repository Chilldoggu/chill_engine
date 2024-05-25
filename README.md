# About
This project is an OpenGL renderer that I'm developing as I learn more about graphic programming. As I continued to study, the renderer grew significantly, so I decided to save and share it on GitHub. The renderer currently lacks a GUI and serves solely as an API for quickly drawing shapes and creating effects using OpenGL. Many uniform names are hard-coded and need to be correctly named in GLSL shaders. Additionally, files such as textures should be stored in the `assets` directory.

# Technologies
- **Glad**: OpenGL loader for OpenGL core profile version 3.3 functions. Although I've included my own generated loader, you will need to generate and overwrite it to match your specific platform requirements.
- **GLM**: Used for transformations and GLSL type compatibility.
- **GLFW3**: Manages window creation and input handling.

# Project Structure
The project directory should be organized as follows:
```
.
├── CMakeLists.txt
├── project_source_files
├── assets
│   └── ...
└── chill_engine
    └── ...
```
