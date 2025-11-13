cmake_minimum_required(VERSION 3.16)
project(game VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OUT ${CMAKE_PROJECT_NAME})
file(GLOB_RECURSE SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)
add_executable(${OUT} ${SRC_FILES})

file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

# sdl
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/sdl)
target_include_directories(${OUT} PRIVATE ${CMAKE_SOURCE_DIR}/extern/sdl/include)

# glm
target_include_directories(${OUT} PRIVATE ${CMAKE_SOURCE_DIR}/extern/glm)

# glad
add_library(glad ${CMAKE_SOURCE_DIR}/extern/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/extern/glad/include)

# stb_image
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

# link ImGui to sdl and glad
target_link_libraries(imgui PUBLIC SDL3::SDL3 glad)

# opengl
find_package(OpenGL REQUIRED)

target_link_libraries(${OUT} PRIVATE
    imgui
    glad
    SDL3::SDL3
    OpenGL::GL
)

target_include_directories(${OUT} PRIVATE ${CMAKE_SOURCE_DIR}/include)

