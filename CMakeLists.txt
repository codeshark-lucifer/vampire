cmake_minimum_required(VERSION 4.2.0)
project(Hades LANGUAGES C CXX)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/freetype)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad)

add_executable(app
    src/main.cpp
    src/core/game.cpp
    src/core/gl_renderer.cpp
    src/core/utils.cpp
    src/core/shader.cpp
    src/core/global_vars.cpp
    src/platform/win32.cpp
)

# if(WIN32)
#     # Build as a GUI application (no console window) on Windows
#     set_target_properties(app PROPERTIES WIN32_EXECUTABLE TRUE)
# endif()

target_include_directories(app PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(app PRIVATE
    user32
    opengl32
    gdi32
    dwmapi
    glad
    freetype
)
