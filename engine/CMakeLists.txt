# ---------------------------------------------------------------------------
# engine - core static library
# ---------------------------------------------------------------------------

set(CATALYST_ENGINE_ANIMATION_SOURCES
  src/animation/AnimationLibrary.cpp
)

set(CATALYST_ENGINE_ASSET_SOURCES
  src/assets/AssetModule.cpp

  src/assets/builtin/BuiltinShaders.cpp

  src/assets/cache/AssetCache.cpp
  src/assets/cache/AssetState.cpp

  src/assets/core/AssetHandle.cpp
  src/assets/core/AssetRecord.cpp
  src/assets/core/AssetType.cpp
  src/assets/core/ImportError.cpp

  src/assets/finalizer/AnimationFinalizer.cpp
  src/assets/finalizer/AssetFinalizer.cpp
  src/assets/finalizer/FontFinalizer.cpp
  src/assets/finalizer/ShaderFinalizer.cpp
  src/assets/finalizer/SoundFinalizer.cpp
  src/assets/finalizer/Texture2DFinalizer.cpp
  src/assets/finalizer/TextureAtlasFinalizer.cpp

  src/assets/importer/ImporterRegistry.cpp

  src/assets/importer/animation/AnimationImporter.cpp

  src/assets/importer/font/FontImportSettings.cpp
  src/assets/importer/font/FontImporter.cpp

  src/assets/importer/shader/ShaderImportSettings.cpp
  src/assets/importer/shader/ShaderImporter.cpp

  src/assets/importer/sound/SoundImporter.cpp

  src/assets/importer/texture2d/Texture2DImportSettings.cpp
  src/assets/importer/texture2d/Texture2DImporter.cpp

  src/assets/importer/texture-atlas/TextureAtlasBuilder.cpp
  src/assets/importer/texture-atlas/TextureAtlasImporter.cpp
  src/assets/importer/texture-atlas/TextureAtlasImportSettings.cpp

  src/assets/manager/AssetManagerEditor.cpp
  src/assets/manager/AssetManagerRuntime.cpp

  src/assets/persistence/AtomicFileWriter.cpp
  src/assets/persistence/RegistrySerializer.cpp
  src/assets/persistence/SidecarSerializer.cpp

  src/assets/watcher/AssetWatcher.cpp
  src/assets/watcher/FileEvent.cpp
)

set(CATALYST_ENGINE_AUDIO_SOURCES
  src/audio/AudioEngine.cpp
  src/audio/MusicPlayer.cpp
)

set(CATALYST_ENGINE_CORE_SOURCES
  src/core/Game.cpp
  src/core/GameConfigSerializer.cpp
  src/core/GameStateManager.cpp
  src/core/DevCapture.cpp
)

set(CATALYST_ENGINE_EDITOR_SOURCES
  src/editor/AssetEditor.cpp
)

set(CATALYST_ENGINE_GRAPHICS_SOURCES
  src/graphics/Renderer.cpp
  src/graphics/TextRenderer.cpp

  src/graphics/gl/Shader.cpp
  src/graphics/gl/SwapChain.cpp

  src/graphics/lighting/LightRenderer.cpp
  src/graphics/lighting/LightTileCuller.cpp

  src/graphics/pipeline/FullscreenQuad.cpp
  src/graphics/pipeline/RenderTarget.cpp

  src/graphics/sprite/SpriteBatch.cpp
  src/graphics/sprite/SpriteDrawList.cpp

  src/graphics/stb/stbi_image_impl.cpp
  src/graphics/stb/stb_truetype_impl.cpp

  src/graphics/text/Font.cpp
  src/graphics/text/TextLayout.cpp

  src/graphics/texture/Texture.cpp
)

set(CATALYST_ENGINE_INPUT_SOURCES
  src/input/InputSystem.cpp
)

set(CATALYST_ENGINE_MATH_SOURCES
  src/math/Rect.cpp
)

set(CATALYST_ENGINE_PLATFORM_SOURCES
  src/platform/Display.cpp
  src/platform/Window.cpp
)

set(CATALYST_ENGINE_RESOURCE_SOURCES
  src/resources/AnimationAsset.cpp
  src/resources/FontAsset.cpp
  src/resources/ShaderAsset.cpp
  src/resources/Texture2DAsset.cpp
  src/resources/TextureAsset.cpp
  src/resources/TextureAtlasAsset.cpp
)

set(CATALYST_ENGINE_SCENE_SOURCES
  src/scene/Scene.cpp
  src/scene/SceneSerializer.cpp
)

set(CATALYST_ENGINE_UTIL_SOURCES
  src/utils/Logger.cpp
)

option(CATALYST_RHI_BACKEND "RHI backend" "gl") # gl | vulkan

set(CATALYST_RHI_SOURCES
  src/rhi/Device.cpp
  src/rhi/SwapChain.cpp
  src/rhi/Texture.cpp
  src/rhi/Sampler.cpp
  src/rhi/Buffer.cpp
  src/rhi/ShaderStages.cpp
  src/rhi/Pipeline.cpp
  src/rhi/BindGroupLayout.cpp
  src/rhi/BindGroup.cpp
  src/rhi/RenderPass.cpp
  src/rhi/CommandList.cpp
  src/rhi/RingBuffer.cpp
)

if (CATALYST_RHI_BACKEND STREQUAL "gl")
  target_sources(engine PRIVATE
    ${CATALYST_RHI_SOURCES}
    src/rhi/gl/gl_device.cpp
    src/rhi/gl/gl_swapChain.cpp
  )
  target_compile_definitions(engine PRIVATE CATALYST_RHI_GL)
endif()


add_library(engine STATIC
  ${CATALYST_ENGINE_ANIMATION_SOURCES}
  ${CATALYST_ENGINE_ASSET_SOURCES}
  ${CATALYST_ENGINE_AUDIO_SOURCES}
  ${CATALYST_ENGINE_CORE_SOURCES}
  ${CATALYST_ENGINE_EDITOR_SOURCES}
  ${CATALYST_ENGINE_GRAPHICS_SOURCES}
  ${CATALYST_ENGINE_INPUT_SOURCES}
  ${CATALYST_ENGINE_MATH_SOURCES}
  ${CATALYST_ENGINE_PLATFORM_SOURCES}
  ${CATALYST_ENGINE_RESOURCE_SOURCES}
  ${CATALYST_ENGINE_SCENE_SOURCES}
  ${CATALYST_ENGINE_UTIL_SOURCES}
)

# Public include root: consumers use <engine/core/Logger.hpp>
target_include_directories(engine
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(engine PUBLIC cxx_std_20)

target_compile_definitions(engine PUBLIC
  GLFW_INCLUDE_NONE=1
  $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:CATALYST_EDITOR>
)

target_link_libraries(engine
  PUBLIC
    glm
    glfw
    glad
    nlohmann_json::nlohmann_json
    field_reflection::field_reflection
  PRIVATE
    imgui
    stb_image
    miniaudio
)

# ---------------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------------
if(GAME_WARNING_FLAGS)
  target_compile_options(engine PRIVATE ${GAME_WARNING_FLAGS})
endif()


add_library(rhi_headers_check OBJECT src/rhi/headers_check.cpp)
target_include_directories(rhi_headers_check PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_features(rhi_headers_check PRIVATE cxx_std_20)
if(GAME_WARNING_FLAGS)
  target_compile_options(rhi_headers_check PRIVATE ${GAME_WARNING_FLAGS})
endif()

# ---------------------------------------------------------------------------
# Built-in shader embedding
# ---------------------------------------------------------------------------
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CatalystShaders.cmake")
catalyst_embed_shaders(engine)
