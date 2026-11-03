#version 450 core

layout(location = 0) in vec2 aCorner; // (-0.5,-0.5)..(0.5,0.5)
layout(location = 1) in vec2 iPosition;
layout(location = 2) in vec2 iScale;
layout(location = 3) in float iRotation;
layout(location = 4) in vec4 iColor;
layout(location = 5) in vec4 iUVRect; // (minX, minY, maxX, maxY)

uniform mat4 uViewProjection;

out vec2 vUV;
out vec4 vColor;

void main() {
  float c = cos(iRotation);
  float s = sin(iRotation);

  vec2 local = aCorner * iScale;
  vec2 rotated = vec2(local.x * c - local.y * s, local.x * s + local.y * c);

  vec2 world = rotated + iPosition;

  vec2 uvT = aCorner + 0.5;

  vUV = mix(iUVRect.xy, iUVRect.zw, uvT);
  vColor = iColor;

  gl_Position = uViewProjection * vec4(world, 0.0, 1.0);
}
