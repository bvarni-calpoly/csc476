#version 330 core

#define MAX_LIGHTS 10

struct PointLight
{
  vec3 position;
  vec3 color;
  vec3 intensity;
};

uniform sampler2D Texture0;
uniform float MatShine;

uniform int flip;

uniform vec3 portalNormal;
uniform vec3 portalPos;
uniform int useSlicing;
uniform vec3 lightPos;

uniform PointLight lights[MAX_LIGHTS];
uniform int numActiveLights;

in vec3 worldPos;
in vec2 vTexCoord;

out vec4 Outcolor;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;

in vec3 EPos; //position of the vertex in camera space

void main() {
  vec4 texColor0 = texture(Texture0, vTexCoord);
  vec3 normal = normalize(fragNor);

  if (flip < 1)
  	normal *= -1.0f;
  
  // Lighting
  vec3 ambient = vec3(0.05f);
  vec3 accumulatedLight = vec3(0.0f);

  for (int i = 0; i < numActiveLights; i++)
  {
    //vec3 light = normalize(lightDir);
    vec3 light = vec3(0.0f);
    vec3 lightColor = vec3(1.0f);
    
    if (i == 0)
    {
      light = normalize(lightDir);
      lightColor = vec3(0.0, 0.5, 1.0);
    } else {
      light = normalize(lightPos - worldPos); // Light source at player
      lightColor = vec3(1.0, 1.0, 1.0);
    }

    float dC = max(0.0f, dot(normal, light));

    accumulatedLight += dC * lightColor;
  }
  
  Outcolor = vec4(accumulatedLight * texColor0.xyz, 1.0);
  // Outcolor = vec4(dC * texColor0.xyz + ambient, 1.0);
  
  //to confirm texture coordinates
  // Outcolor = vec4(vTexCoord.x, vTexCoord.y, 0, 0);
  
  // fullbright
  // Outcolor = vec4(texColor0.xyz, 1.0);

  if (useSlicing > 0)
  {
    float sliceSide = dot(portalNormal, worldPos - portalPos);
    if (sliceSide < 0.0f) discard;
  }
}