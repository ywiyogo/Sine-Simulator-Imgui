# Module "core"

add_library(core_logic OBJECT
    CoreLogic.cpp
)
target_include_directories(core_logic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(core_logic PUBLIC cxx_std_20)

# Add compiler warning flags
target_compile_options(core_logic PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall>
    $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
)

target_link_libraries(core_logic fmt::fmt)
