#version 450 core

in vec2 vUV;

uniform mat4 uInViewProjection;
uniform vec3 uAmbientColor;
uniform int uPointLightCount;
uniform int uDirectionalLightCount;
uniform int uSpotLightCount;

uniform uint uTileCountX;
uniform uint uTileCountY;
uniform vec2 uTileWorldSize;
uniform vec2 uWorldOrigin;

struct PointLight {
  vec4 position;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec4 attenuation;
};

struct DirectionalLight {
  vec4 direction;
  vec4 color;
};

struct SpotLight {
  vec4 position;
  vec4 direction;
  vec4 color;
  vec4 attenuation;
  vec4 cone;
};

layout(std140, binding = 0) uniform PointLightBlock {
  PointLight uPointLights[64];
};

layout(std140, binding = 1) uniform DirectionalLightBlock {
  DirectionalLight uDirectionalLights[4];
};

layout(std140, binding = 2) uniform SpotLightBlock {
  SpotLight uSpotLights[16];
};

layout(std430, binding = 3) readonly buffer TileOffsets {
  uint uTileOffsets[];
};

layout(std430, binding = 4) readonly buffer TileIndices {
  uint uTileIndices[];
};

out vec4 FragColor;

vec2 ReconstructWorld(vec2 uv) {
  vec4 ndc = vec4(uv * 2.0 - 1.0, 0.0, 1.0);
  vec4 worldH = uInViewProjection * ndc;
  return worldH.xy / worldH.w;
}

void main() {
  // Reconstruct world position from NDC.
  vec2 world = ReconstructWorld(vUV);

  vec3 light = uAmbientColor;

  for (int i = 0; i < uDirectionalLightCount; ++i) {
    light += uDirectionalLights[i].color.rgb;
  }

  // Tile lookup for point lights
  ivec2 tile = ivec2(floor((world - uWorldOrigin) / uTileWorldSize));
  tile = clamp(tile, ivec2(0), ivec2(uTileCountX - 1u, uTileCountY - 1u));
  uint t = uint(tile.y) * uTileCountX + uint(tile.x);
  uint start = uTileOffsets[t];
  uint end = uTileOffsets[t + 1u];

  for (uint i = start; i < end; ++i) {
    uint li = uTileIndices[i];
    PointLight L = uPointLights[li];
    float d = length(L.position.xy - world);
    float att = 1.0 / (L.attenuation.x + L.attenuation.y * d + L.attenuation.z * d * d);
    light += att * (L.ambient.rgb + L.diffuse.rgb);
  }
  // for (int i = 0; i < uPointLightCount; ++i) {
  //   PointLight L = uPointLights[i];
  //   float d = length(L.position.xy - world);
  //   float att = 1.0 / (L.attenuation.x + L.attenuation.y * d
  //         + L.attenuation.z * d * d);
  //   light += att * (L.ambient.rgb + L.diffuse.rgb);
  // }

  for (int i = 0; i < uSpotLightCount; ++i) {
    vec2 toFrag = world - uSpotLights[i].position.xy;
    float d = max(length(toFrag), 1e-4);
    vec2 L = toFrag / d;
    float c = dot(L, normalize(uSpotLights[i].direction.xy));
    float t2 = clamp((c - uSpotLights[i].cone.y) / max(uSpotLights[i].cone.x - uSpotLights[i].cone.y, 1e-4), 0.0, 1.0);

    if (t2 <= 0.0) continue;

    float att = 1.0 / (uSpotLights[i].attenuation.x
          + uSpotLights[i].attenuation.y * d
          + uSpotLights[i].attenuation.z * d * d);

    light += t2 * att * uSpotLights[i].color.rgb;
  }

  FragColor = vec4(light, 1.0);
}
