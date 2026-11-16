#version 450 core

in vec2 vUV;

uniform sampler2D uScene;
uniform sampler2D uLight;
uniform int uMode;

out vec4 FragColor;

void main() {
  vec3 scene = texture(uScene, vUV).rgb;
  vec3 light = texture(uLight, vUV).rgb;
  vec3 result = scene * light;

  if (uMode == 1) result = scene;
  else if (uMode == 2) result = light;

  FragColor = vec4(result, 1.0);
}
