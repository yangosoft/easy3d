cmake_minimum_required(VERSION 3.18)
project(easy3d LANGUAGES CXX)

# Use C++23
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_TESTING OFF CACHE BOOL "Build the tests")

set(CMAKE_VERBOSE_MAKEFILE ON)

include(FetchContent)

# FetchContent_Declare(
#       assimp
#       GIT_REPOSITORY https://github.com/assimp/assimp.git
#       GIT_TAG v6.0.4
#       GIT_PROGRESS TRUE
# )
# FetchContent_MakeAvailable(assimp)



set(MANIFOLD_EXPORT ON CACHE BOOL "" FORCE)
set(MANIFOLD_EXPORT OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
      manifold
      GIT_REPOSITORY https://github.com/elalish/manifold.git
      GIT_TAG v3.3.2
      GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(manifold)


FetchContent_Declare(
      imgui
      GIT_REPOSITORY https://github.com/ocornut/imgui.git
      GIT_TAG v1.92.5
      GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(imgui)


FetchContent_Declare(glad GIT_REPOSITORY https://github.com/Dav1dde/glad.git 
GIT_TAG v2.0.8)
FetchContent_MakeAvailable(glad)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_MakeAvailable(glfw)



find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
# find_package(OPENGL REQUIRED)
find_package(OpenGL REQUIRED)

 set(IMGUI_INCLUDE_DIR ${imgui_SOURCE_DIR}/)
    file(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp)
    file(GLOB IMGUI_HEADERS ${imgui_SOURCE_DIR}/*.h)
    add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS} 
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
    )
    add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
    target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${GLAD_INCLUDE_DIR})
    target_link_libraries(imgui ${OPENGL_LIBRARIES} glfw SDL2::SDL2)

add_library(sengi 
src/sengi/sengi.cpp
src/sengi/node.cpp
src/sengi/scene.cpp
include/sengi/sengi.hpp
include/sengi/node.hpp
include/sengi/scene.hpp)

target_include_directories(sengi PRIVATE include)

add_executable(
  easy3d
  src/main2.cpp
  src/render.cpp
  include/easy3d.hpp
  include/main.hpp
  include/easy3d/utils.hpp
  include/easy3d/render.hpp
    
    

)


target_include_directories(easy3d PRIVATE include ${IMGUI_INCLUDE_DIR})
target_link_libraries(easy3d PUBLIC manifold imgui SDL2::SDL2 GLEW::GLEW OpenGL::GL OpenGL::GLU sengi )

# Testing

if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()