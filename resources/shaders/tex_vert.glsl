#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
// uniform vec3 lightPos;

out vec3 worldPos;
out vec3 fragNor;
out vec3 lightDir;
out vec3 EPos;
out vec2 vTexCoord;

void main() {
  // Calculate world positions
  vec4 wPos = M * vec4(vertPos, 1.0);
  worldPos = wPos.xyz;

  // Clip space
  gl_Position = P * V * wPos;

  // Transform normal into world space
  fragNor = (M * vec4(vertNor, 0.0)).xyz;
  
  // World space lighting
  // lightDir = lightPos - worldPos;

  EPos = vec3(1); //PULLED for release
  
  /* pass through the texture coordinates to be interpolated */
  vTexCoord = vertTex;
}