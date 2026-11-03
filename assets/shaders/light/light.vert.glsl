#version 450 core

layout(location = 0) in vec2 aPos;
// layout(location = 1) in vec3 aNormal;
// layout(location = 2) in vec2 aTexCoords;
//
// layout(location = 0) out vec3 vFragPos;
// layout(location = 1) out vec3 vNormal;
// layout(location = 2) out vec2 vTexCoords;

// uniform mat4 uModel;
// uniform mat4 uView;
// uniform mat4 uProjection;
// uniform mat3 uNormalMatrix;

out vec2 vUV;

void main() {
  // vec4 worldPos = uModel * vec4(aPos, 1.0);

  // vFragPos = vec3(uModel * vec4(aPos, 1.0));
  // vNormal = normalize(uNormalMatrix * aNormal);
  // vTexCoords = aTexCoords;

  vUV = aPos * 0.5 + 0.5;
  gl_Position = vec4(aPos, 0.0, 1.0);
}
