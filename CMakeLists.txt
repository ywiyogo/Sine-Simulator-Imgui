cmake_minimum_required(VERSION 3.30)
project(sine-simulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
# Handle fmt dependency
if(EMSCRIPTEN)
    # For web builds, get fmt through FetchContent
    include(FetchContent)
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 9.1.0 # Using an older version known to work with Emscripten
    )
    FetchContent_MakeAvailable(fmt)

    set(FMT_LIBRARIES fmt::fmt)
else()
    # For native builds, use system fmt
    find_package(fmt REQUIRED)
    set(FMT_LIBRARIES fmt::fmt)
endif()

if(EMSCRIPTEN)

    # Set Emscripten specific flags

    set(EM_FLAGS "\
        -s USE_SDL=2 \
    ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EM_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EM_FLAGS}")

    # Emscripten provides its own SDL2
    set(SDL2_INCLUDE_DIRS "")
    set(SDL2_LIBRARIES "")
else()
    find_package(SDL2 REQUIRED)
endif()

# Add external dependencies (including ImGui)
add_subdirectory(external)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Application.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}  # For wrapper headers
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SDL2_LIBRARIES}
    imgui
    imgui_sdl_backends
    ${FMT_LIBRARIES}
)

# Web build specific settings
if(EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "\
            ${EM_FLAGS} \
            ${EM_DEBUG_FLAGS} \
            --shell-file ${CMAKE_SOURCE_DIR}/web/shell.html"
    )
endif()
