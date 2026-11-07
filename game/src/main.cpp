#include "engine/asset/AssetManager.hpp"
#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/asset/core/ImportRequest.hpp"
#include "engine/asset/finalizer/AssetFinalizer.hpp"
#include "engine/asset/importer/ImporterRegistry.hpp"
#include "engine/utils/Logger.hpp"
#include <cassert>

#include <GLFW/glfw3.h>
#include <glad/gl.h>

// #include "game/KingdomRushGame.hpp"

int main(int /*argc*/, char ** /*argv*/) {
  if (!glfwInit()) {
    engine::Logger::Error("GLWF INIT FAILED");
    return 0;
  }

  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(1, 1, "asset-test", nullptr, nullptr);
  if (!window) {
    engine::Logger::Error("GLFW CREATE WINDOW FAILED");
    return 0;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGL(glfwGetProcAddress)) {
    engine::Logger::Error("gladLoadGL failed");
    glfwDestroyWindow(window);
    return 0;
  }

  assert(glGenTextures != nullptr);

  engine::asset::ImporterRegistry registry;
  engine::asset::AssetManager::RegisterDefaultImporters(registry);

  struct Case {
    const char *ext;
    const char *path;
    engine::asset::AssetType expectedType;
  };
  const Case cases[] = {
      {".png", "assets/test/hero.png", engine::asset::AssetType::Texture2D},
      {".ttf", "assets/test/Roboto.ttf", engine::asset::AssetType::Font},
      {".wav", "assets/test/click.wav", engine::asset::AssetType::Sound},
      {".anim", "assets/test/walk.anim", engine::asset::AssetType::Animation},
  };

  for (const auto &c : cases) {
    auto *importer = registry.FindForExtension(c.ext);
    assert(importer && "no importer found");

    engine::asset::ImportRequest req{c.path};
    auto imported = importer->Import(req);
    auto handle = engine::asset::AssetHandle::Generate();

    imported.Match(
        [&](engine::asset::ImportedAssetDataPtr &data) {
          auto asset =
              engine::asset::AssetFinalizer::Finalize(handle, std::move(data));
        },
        [&](const engine::asset::ImportError &err) {
          engine::Logger::Error("[Imported] import failed: %s",
                                err.ToString().c_str());
        });
    if (imported.IsError()) {
      engine::Logger::Error("Import failed for '%s': %s", c.path,
                            imported.Error().ToString().c_str());
      assert(false && "cpu import failed");
      continue;
    }

    assert(imported.Value()->GetAssetType() == c.expectedType);

    auto asset = engine::asset::AssetFinalizer::Finalize(
        handle, std::move(imported.Value()));
    assert(asset && "finalize failed");
    assert(asset->Handle() == handle && "handle not stamped");
    assert(asset->Type() == c.expectedType);

    engine::Logger::Info("OK: %s -> %s", c.ext, c.path);
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  // KingdomRushGame game;
  // game.Run();
  // return 0;
}
