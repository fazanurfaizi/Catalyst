#pragma once

#include "engine/math/Color.hpp"
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace glm {

inline void to_json(nlohmann::json &j, const glm::vec2 &v) {
  j = nlohmann::json::array({v.x, v.y});
}
inline void from_json(const nlohmann::json &j, glm::vec2 &v) {
  if (j.is_array() && j.size() >= 2) {
    v.x = j[0].get<float>();
    v.y = j[1].get<float>();
  }
}

inline void to_json(nlohmann::json &j, const glm::vec3 &v) {
  j = nlohmann::json::array({v.x, v.y, v.z});
}
inline void from_json(const nlohmann::json &j, glm::vec3 &v) {
  if (j.is_array() && j.size() >= 3) {
    v.x = j[0].get<float>();
    v.y = j[1].get<float>();
    v.z = j[2].get<float>();
  }
}

inline void to_json(nlohmann::json &j, const glm::vec4 &v) {
  j = nlohmann::json::array({v.x, v.y, v.z, v.w});
}
inline void from_json(const nlohmann::json &j, glm::vec4 &v) {
  if (j.is_array() && j.size() >= 4) {
    v.x = j[0].get<float>();
    v.y = j[1].get<float>();
    v.z = j[2].get<float>();
    v.w = j[3].get<float>();
  }
}

} // namespace glm

namespace engine::math {

inline void to_json(nlohmann::json &j, const Color &c) {
  j = nlohmann::json::array({c.r, c.g, c.b, c.a});
}

inline void from_json(const nlohmann::json &j, Color &c) {
  if (j.is_array() && j.size() >= 4) {
    c.r = j[0].get<float>();
    c.g = j[1].get<float>();
    c.b = j[2].get<float>();
    c.a = j[3].get<float>();
  }
}

} // namespace engine::math
