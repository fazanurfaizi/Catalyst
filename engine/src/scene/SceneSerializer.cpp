// #include "engine/scene/SceneSerializer.hpp"
//
// #include "engine/core/Logger.hpp"
// #include "engine/graphics/TextureAtlas.hpp"
// #include "engine/scene/Scene.hpp"
//
// #include <fstream>
// #include <nlohmann/json.hpp>
//
// using json = nlohmann::json;
//
// namespace engine {
//
// bool SceneSerializer::Save(const Scene &scene, const TextureAtlas &atlas,
//                            const std::string &filePath) {
//   json root;
//   root["entities"] = json::array();
//
//   scene.ForEach([&](Entity e, const Transform &t, const Sprite &s) {
//     json j_entity;
//     j_entity["name"] = scene.NameOf(e);
//
//     j_entity["transform"] = {
//         {"px", t.position.x}, {"py", t.position.y}, {"sx", t.scale.x},
//         {"sy", t.scale.y},    {"rot", t.rotation},
//     };
//
//     j_entity["sprite"] = {
//         {"color", {s.color.r, s.color.g, s.color.b, s.color.a}},
//         {"layer", static_cast<int>(s.layer)},
//         {"ySortKey", s.ySortKey},
//         {"region", s.regionName},
//     };
//
//     root["entities"].push_back(j_entity);
//   });
//
//   std::ofstream file(filePath);
//   if (!file.is_open()) {
//     Logger::Error("[SceneSerializer] Faield to open for writing: %s",
//                   filePath.c_str());
//     return false;
//   }
//
//   file << root.dump(2);
//   Logger::Info("[SceneSerializer] Saved %zu entities to %s", scene.Size(),
//                filePath.c_str());
//   return true;
// }
//
// bool SceneSerializer::Load(Scene &scene, const TextureAtlas &atlas,
//                            const std::string &filePath) {
//   std::ifstream file(filePath);
//   if (!file.is_open()) {
//     Logger::Error("[SceneSerializer] Failed to open for reading: %s",
//                   filePath.c_str());
//     return false;
//   }
//
//   json root;
//   try {
//     file >> root;
//   } catch (const json::exception &e) {
//     Logger::Error("[SceneSerializer] JSON parse error: %s", e.what());
//     return false;
//   }
//
//   if (!root.contains("entities") || !root["entities"].is_array()) {
//     Logger::Error("[SceneSerializer] Invalid schema: missing 'entities'");
//     return false;
//   }
//
//   scene.Clear();
//
//   for (const auto &j_e : root["entities"]) {
//     std::string name = j_e.value("name", "Entity");
//     Entity e = scene.CreateEntity(name);
//
//     // Transform
//     const auto &j_t = j_e["transform"];
//     auto &t = scene.TransformOf(e);
//     t.position.x = j_t.value("px", 0.0f);
//     t.position.y = j_t.value("py", 0.0f);
//     t.scale.x = j_t.value("sx", 1.0f);
//     t.scale.y = j_t.value("sy", 1.0f);
//     t.rotation = j_t.value("rot", 0.0f);
//
//     // Sprite
//     const auto &j_s = j_e["sprite"];
//     auto &s = scene.SpriteOf(e);
//     s.color.r = j_s["color"][0].get<float>();
//     s.color.g = j_s["color"][1].get<float>();
//     s.color.b = j_s["color"][2].get<float>();
//     s.color.a = j_s["color"][3].get<float>();
//     s.layer = static_cast<RenderLayer>(j_s.value("layer", 3));
//     s.ySortKey = j_s.value("ySortKey", 0.0f);
//     s.regionName = j_s.value("region", "");
//
//     if (!s.regionName.empty()) {
//       if (const auto *region = atlas.GetRegion(s.regionName)) {
//         s.texture = &atlas.GetTexture();
//         s.uvMin = region->uvMin;
//         s.uvMax = region->uvMax;
//       } else {
//         Logger::Warn("[SceneSerializer] Unknown region '%s' for entity '%s'",
//                      s.regionName.c_str(), name.c_str());
//       }
//     }
//   }
//
//   Logger::Info("[SceneSerializer] Loaded %zu entities from %s", scene.Size(),
//                filePath.c_str());
//   return true;
// }
//
// } // namespace engine
