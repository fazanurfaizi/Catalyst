#pragma once

#include "engine/graphics/Shader.hpp"
#include "engine/graphics/Texture.hpp"

namespace engine::graphics {

struct LightMaterial {
  const graphics::Texture *diffuse = nullptr;
  const graphics::Texture *specular = nullptr;
  float shininess = 32.0f;
  float specularStrength = 0.25f;

  void Apply(graphics::Shader &shader) const {
    shader.SetInt("uMaterial.diffuseMap", 0);
    shader.SetInt("uMaterial.hasSpecularMap", specular ? 1 : 0);
    shader.SetInt("uMaterial.specularMap", 1);
    shader.SetFloat("uMaterial.shininess", shininess);
    shader.SetFloat("uMaterial.specularStrength", specularStrength);

    if (diffuse)
      diffuse->Bind();
    if (specular)
      specular->Bind();
    else if (diffuse)
      diffuse->Bind(1);
  }
};

} // namespace engine::graphics
