cmake_minimum_required(VERSION 4.2) 
project(vampire LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

add_subdirectory(vendor/glfw)

if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "VULKAN_SDK environment variable is not set. Please set it to your Vulkan SDK path.")
endif()

include_directories("$ENV{VULKAN_SDK}/Include")
link_directories("$ENV{VULKAN_SDK}/Lib")

find_package(Vulkan REQUIRED)

add_executable(app 
    src/main.cpp
)

target_include_directories(app PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include
)

target_link_libraries(app 
    PRIVATE
    Vulkan::Vulkan
    glfw    # or glfw::GLFW depending on the version in vendor
    user32
    gdi32   # Often needed for Win32 graphics apps
    shell32
    dwmapi 
)

# if(WIN32)
#     # Build as a GUI application (no console window) on Windows
#     set_target_properties(app PROPERTIES WIN32_EXECUTABLE TRUE)
# endif()