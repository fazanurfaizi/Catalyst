#pragma once

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/manager/IAssetManager.hpp"

namespace engine::editor {

struct AssetBrowserState {
  char filter[128] = {};
  char importPath[256] = "textures/tower.png";
  char importName[128] = {};
  engine::asset::AssetHandle selected{};
};

void DrawAssetBrowser(engine::asset::IAssetManager &assets,
                      AssetBrowserState &state);

} // namespace engine::editor
