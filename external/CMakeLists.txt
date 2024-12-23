include(FetchContent)

# Declare ImGui dependency
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking # Specify a version tag for stability
    GIT_SHALLOW TRUE # Only fetch the specified commit
)

# Make ImGui available
FetchContent_MakeAvailable(imgui)

# Generate a custom header for SDL include
if(EMSCRIPTEN)
    file(WRITE ${CMAKE_BINARY_DIR}/imgui_sdl_include.h
        "#include <SDL.h>
        #include \"imgui.h\"
    "    )
else()
    file(WRITE ${CMAKE_BINARY_DIR}/imgui_sdl_include.h
        "#include <SDL2/SDL.h>
        #include \"imgui.h\"
    "    )
endif()

# Create wrapper for SDL2 backend
file(WRITE ${CMAKE_BINARY_DIR}/imgui_impl_sdl2_wrapper.h
    "${SDL_INCLUDE}
    #include \"imgui.h\"
    #include \"${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.h\"
")

# Create wrapper for SDL2 Renderer backend
file(WRITE ${CMAKE_BINARY_DIR}/imgui_impl_sdlrenderer2_wrapper.h
    "${SDL_INCLUDE}
    #include \"imgui.h\"
    #include \"${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.h\"
")

# Create ImGui library with our specific requirements
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

# Create a separate library for SDL backends
add_library(imgui_sdl_backends STATIC
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${SDL2_INCLUDE_DIRS}
)

target_include_directories(imgui_sdl_backends PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR} # For wrapper headers
)

target_link_libraries(imgui_sdl_backends PUBLIC
    imgui
    ${SDL2_LIBRARIES}
)

# Set up compile definitions
if(EMSCRIPTEN)
    target_compile_definitions(imgui PUBLIC
        IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
        IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
    )
endif()
