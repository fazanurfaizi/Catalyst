add_library(engine_resources STATIC)

target_sources(engine_resources PRIVATE
  AnimationAsset.cpp
  FontAsset.cpp
  ShaderAsset.cpp
  TextureAsset.cpp
  Texture2DAsset.cpp
  TextureAtlasAsset.cpp
)

target_include_directories(engine_resources PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_resources
  PUBLIC  engine_utils engine_math engine_graphics engine_assets
)

target_include_directories(engine_resources PRIVATE
  "${CMAKE_SOURCE_DIR}/vendor/stb-image/include"
)

if(CATALYST_WARNING_FLAGS)
  target_compile_options(engine_resources PRIVATE ${CATALYST_WARNING_FLAGS})
endif()
