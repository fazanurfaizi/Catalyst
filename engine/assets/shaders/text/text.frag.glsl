#version 450 core

in vec2 vTexCoords;
out vec4 vColor;

uniform sampler2D uText;
uniform vec4 uTextColor;

void main() {
  float alpha = texture(uText, vTexCoords).r;
  vColor = vec4(uTextColor.rgb, uTextColor.a * alpha);
}
