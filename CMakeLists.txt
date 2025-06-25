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
        -s FULL_ES3=1 \
        -s MIN_WEBGL_VERSION=2 \
        -s MAX_WEBGL_VERSION=2 \
    ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EM_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EM_FLAGS}")

    # Emscripten provides its own SDL2
    set(SDL2_INCLUDE_DIRS "")
    set(SDL2_LIBRARIES "")
else()
    find_package(SDL2 REQUIRED)
endif()

add_subdirectory(src/core)
add_subdirectory(src/ui)

# Add external dependencies (including ImGui)
add_subdirectory(external)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    core_logic
    gui
)

# Add compiler warning flags (most modern target-specific approach)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall>
    $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
)

# Web build specific settings
if(EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "\
            ${EM_FLAGS} \
            ${EM_DEBUG_FLAGS} \
            --preload-file ${CMAKE_SOURCE_DIR}/web/fonts/NotoSans-Regular.ttf@/fonts/NotoSans-Regular.ttf \
            --shell-file ${CMAKE_SOURCE_DIR}/web/shell.html"
)
endif()

find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/*.h"
        "${CMAKE_SOURCE_DIR}/src/*.hpp"
        "${CMAKE_SOURCE_DIR}/src/*.c"
        "${CMAKE_SOURCE_DIR}/include/*.h"
        "${CMAKE_SOURCE_DIR}/include/*.hpp"
)

    add_custom_target(clang_format
        COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${ALL_SOURCE_FILES}
        COMMENT "Running clang-format on specified source files"
)
endif()
