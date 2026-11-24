add_library(engine_utils STATIC)

target_sources(engine_utils PRIVATE Logger.cpp)

target_include_directories(engine_utils PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_compile_features(engine_utils PUBLIC cxx_std_20)

if(CATALYST_WARNING_FLAGS)
  target_compile_options(engine_utils PRIVATE ${CATALYST_WARNING_FLAGS})
endif()
