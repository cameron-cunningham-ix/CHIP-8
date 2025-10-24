# If building, include this in the imgui folder
# ImGui static library + SDL3 backends (SDL_Renderer)
cmake_minimum_required(VERSION 3.16)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(IMGUI_CORE_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)

# SDL3 backends
set(IMGUI_BACKENDS_SRC
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

add_library(imgui STATIC
    ${IMGUI_CORE_SRC}
    ${IMGUI_BACKENDS_SRC}
)

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC SDL3::SDL3)
