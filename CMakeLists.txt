
cmake_minimum_required(VERSION 3.16)
project(CHIP8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")


# This assumes the SDL and imGui sources are available in vendored/
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/imgui EXCLUDE_FROM_ALL)

# Create game executable target
add_executable(CHIP8 main.cpp
    Chip8.cpp
)

# C source files
target_sources(CHIP8 PRIVATE
    Chip8.cpp
    Chip8.h
    Chip8Platform.h
)

target_include_directories(CHIP8 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link to the actual SDL3 library.
target_link_libraries(CHIP8 PRIVATE
    SDL3::SDL3 
    imgui
)