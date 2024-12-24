# Module "path_planner"

add_library(gui OBJECT
    Gui.cpp
)
target_include_directories(gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    ${imgui_BINARY_DIR}
    ${CMAKE_BINARY_DIR} # For wrapper headers
    ${CMAKE_SOURCE_DIR}/src/core/include
)

target_compile_features(gui PUBLIC cxx_std_20)

target_link_libraries(gui
    fmt::fmt
    ${SDL2_LIBRARIES}
    imgui
    imgui_sdl_backends
    ${FMT_LIBRARIES}
)
