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

# Assimp configuration
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable assimp tests")
set(ASSIMP_INSTALL OFF CACHE BOOL "Disable assimp install")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Disable assimp tools")
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "Build zlib")
add_subdirectory(vendor/assimp)

if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "VULKAN_SDK environment variable is not set. Please set it to your Vulkan SDK path.")
endif()

include_directories("$ENV{VULKAN_SDK}/Include")
link_directories("$ENV{VULKAN_SDK}/Lib")

find_package(Vulkan REQUIRED)

set(SHADER_SOURCES
    shaders/shader.vert
    shaders/shader.frag
)

set(SHADER_BINARIES "")
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPIRV_FILE "${CMAKE_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
        COMMAND "$ENV{VULKAN_SDK}/bin/glslc.exe" ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} -o ${SPIRV_FILE}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER} to ${SPIRV_FILE}"
    )
    list(APPEND SHADER_BINARIES ${SPIRV_FILE})
endforeach()

add_executable(app 
    src/main.cpp
    ${SHADER_BINARIES}
)

target_include_directories(app PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp/include
    ${CMAKE_BINARY_DIR}/vendor/assimp/include
)

target_link_libraries(app 
    PRIVATE
    Vulkan::Vulkan
    glfw
    assimp::assimp
    user32
    gdi32
    shell32
    dwmapi 
)
