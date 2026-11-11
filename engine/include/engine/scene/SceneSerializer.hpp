#pragma once

#include "engine/math/MathJson.hpp"
#include "engine/scene/Scene.hpp"

#include <string>

namespace engine::scene {

class SceneSerializer {
public:
  /*
   * @brief Saves the scene to a JSON file.
   */
  static bool Save(const scene::Scene &scene, const std::string &filePath);

  /*
   * @brief Clears the scene and loads entities from a JSON file.
   */
  static bool Load(scene::Scene &scene, const std::string &filePath);
};

} // namespace engine::scene
