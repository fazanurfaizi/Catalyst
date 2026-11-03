#version 450 core

in vec2 vUV;

uniform sampler2D uScene;
uniform sampler2D uLight;

out vec4 FragColor;

void main() {
  vec3 scene = texture(uScene, vUV).rgb;
  vec3 light = texture(uLight, vUV).rgb;
  FragColor = vec4(scene * light, 1.0);
}
