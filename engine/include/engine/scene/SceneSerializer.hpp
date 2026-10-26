// #pragma once
//
// #include <string>
//
// namespace engine {
//
// class Scene;
// class TextureAtlas;
//
// class SceneSerializer {
// public:
//   /*
//    * @brief Saves the scene to a JSON file.
//    */
//   static bool Save(const Scene &scene, const TextureAtlas &atlas,
//                    const std::string &filePath);
//
//   /*
//    * @brief Clears the scene and loads entities from a JSON file.
//    */
//   static bool Load(Scene &scene, const TextureAtlas &atlas,
//                    const std::string &filePath);
// };
//
// } // namespace engine
