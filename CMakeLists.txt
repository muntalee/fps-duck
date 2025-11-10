cmake_minimum_required(VERSION 3.16)
project(game VERSION 1.0)

set(OUT ${CMAKE_PROJECT_NAME})
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(${OUT} ${SOURCES})

# SDL
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/sdl)
target_include_directories(${OUT} PRIVATE ${CMAKE_SOURCE_DIR}/extern/sdl/include)
target_link_libraries(${OUT} PRIVATE SDL3::SDL3)

# GLM (header-only)
target_include_directories(${OUT} PRIVATE ${CMAKE_SOURCE_DIR}/extern/glm)

# GLAD (manual compile)
add_library(glad ${CMAKE_SOURCE_DIR}/extern/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/extern/glad/include)
target_link_libraries(${OUT} PRIVATE glad)

# stb_image (header-only)
target_include_directories(${OUT} PRIVATE ${CMAKE_SOURCE_DIR}/extern/stb)

# ImGui
file(GLOB IMGUI_SRC
    ${CMAKE_SOURCE_DIR}/extern/imgui/*.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/backends/imgui_impl_sdl3.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/backends/imgui_impl_opengl3.cpp
)
add_library(imgui ${IMGUI_SRC})
target_include_directories(imgui PUBLIC
    ${CMAKE_SOURCE_DIR}/extern/imgui
    ${CMAKE_SOURCE_DIR}/extern/imgui/backends
    ${CMAKE_SOURCE_DIR}/extern/sdl/include
    ${CMAKE_SOURCE_DIR}/extern/glad/include
)
target_link_libraries(${OUT} PRIVATE imgui)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${OUT} PRIVATE OpenGL::GL)

target_include_directories(${OUT} PRIVATE ${CMAKE_SOURCE_DIR}/include)

