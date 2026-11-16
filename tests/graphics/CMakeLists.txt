add_executable(golden_main
  golden_main.cpp
  golden_state.cpp
)
target_link_libraries(golden_main PRIVATE engine)
target_include_directories(golden_main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(golden_main PRIVATE
  GOLDEN_ASSET_ROOT="${CMAKE_SOURCE_DIR}"
)

add_executable(test_golden test_golden.cpp)
target_link_libraries(test_golden PRIVATE engine)
target_compile_definitions(test_golden PRIVATE
  GOLDEN_ASSET_ROOT="${CMAKE_SOURCE_DIR}"
)

add_test(
  NAME golden
  COMMAND test_golden $<TARGET_FILE:golden_main>
)
