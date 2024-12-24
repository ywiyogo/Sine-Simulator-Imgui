# Module "path_planner"

add_library(core_logic OBJECT
    CoreLogic.cpp
)
target_include_directories(core_logic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(core_logic PUBLIC cxx_std_20)

target_link_libraries(core_logic fmt::fmt)
