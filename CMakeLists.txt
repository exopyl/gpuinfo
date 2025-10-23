cmake_minimum_required(VERSION 3.15)
project(gpuinfo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)

# --- Librairie GLAD ---
add_library(glad STATIC
        glad/gl.c
        glad/wgl.c    # sur Windows, sinon juste gl.c
)

target_compile_definitions(glad PRIVATE GLAD_GL_IMPLEMENTATION)

# --- Exécutable principal ---
add_executable(${PROJECT_NAME}
        main.cpp
        gpuinfo.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL glad)
