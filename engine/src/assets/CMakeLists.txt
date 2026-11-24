add_library(engine_assets STATIC)

target_sources(engine_assets PRIVATE
  builtin/BuiltinShaders.cpp

  cache/AssetCache.cpp
  cache/AssetState.cpp

  core/AssetHandle.cpp
  core/AssetRecord.cpp
  core/AssetType.cpp
  core/ImportError.cpp

  finalizer/AnimationFinalizer.cpp
  finalizer/AssetFinalizer.cpp
  finalizer/FontFinalizer.cpp
  finalizer/SoundFinalizer.cpp
  finalizer/ShaderFinalizer.cpp
  finalizer/Texture2DFinalizer.cpp
  finalizer/TextureAtlasFinalizer.cpp

  importer/ImporterRegistry.cpp
  importer/animation/AnimationImporter.cpp
  importer/font/FontImportSettings.cpp
  importer/font/FontImporter.cpp
  importer/sound/SoundImporter.cpp
  importer/shader/ShaderImportSettings.cpp
  importer/shader/ShaderImporter.cpp
  importer/texture2d/Texture2DImportSettings.cpp
  importer/texture2d/Texture2DImporter.cpp
  importer/texture-atlas/TextureAtlasImporter.cpp
  importer/texture-atlas/TextureAtlasBuilder.cpp
  importer/texture-atlas/TextureAtlasImportSettings.cpp

  persistence/AtomicFileWriter.cpp
  persistence/RegistrySerializer.cpp
  persistence/SidecarSerializer.cpp

  manager/AssetManagerRuntime.cpp
  manager/AssetManagerEditor.cpp

  stb/stbi_image_impl.cpp
  stb/stb_truetype_impl.cpp

  watcher/AssetWatcher.cpp
  watcher/FileEvent.cpp

  AssetModule.cpp
)

target_include_directories(engine_assets PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_assets
  PUBLIC
    engine_utils
    engine_math
    engine_graphics
    engine_resources
    nlohmann_json::nlohmann_json
  PRIVATE
    glm
    stb_image
    miniaudio
)

if(CATALYST_WARNING_FLAGS)
  target_compile_options(engine_assets PRIVATE ${CATALYST_WARNING_FLAGS})
endif()
