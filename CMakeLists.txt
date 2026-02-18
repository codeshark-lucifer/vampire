cmake_minimum_required(VERSION 3.16)
project(Hades LANGUAGES C CXX)

include(FetchContent)

# -------------------------------------------------------
# FreeType (clone automatically if not present)
# -------------------------------------------------------

set(FREETYPE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/freetype)

if(NOT EXISTS ${FREETYPE_DIR}/CMakeLists.txt)
    message(STATUS "FreeType not found. Cloning from GitHub...")

    execute_process(
        COMMAND git clone https://github.com/freetype/freetype.git ${FREETYPE_DIR}
        RESULT_VARIABLE GIT_RESULT
    )

    if(NOT GIT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone FreeType repository")
    endif()
endif()

add_subdirectory(${FREETYPE_DIR})

# -------------------------------------------------------
# Glad (assumed already present)
# -------------------------------------------------------

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad)

# -------------------------------------------------------
# Compiler settings
# -------------------------------------------------------

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# -------------------------------------------------------
# Executable
# -------------------------------------------------------

add_executable(app
    src/main.cpp
    src/core/game.cpp
    src/core/gl_renderer.cpp
    src/core/utils.cpp
    src/core/shader.cpp
    src/core/global_vars.cpp
    src/platform/win32.cpp
)

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

# if(WIN32)
#     # Build as a GUI application (no console window) on Windows
#     set_target_properties(app PROPERTIES WIN32_EXECUTABLE TRUE)
# endif()
