#version 330 core

// For finding bright objects
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

// Use vec4 for padding
struct PointLight
{
  vec4 position;
  vec4 color;
  vec4 intensity;
};

#define MAX_LIGHTS 20
layout (std140) uniform LightBlock
{
    PointLight lights[MAX_LIGHTS];
    ivec4 numActiveLights;
};

uniform sampler2D Texture0;
uniform float MatShine;

uniform int flip;

uniform vec3 portalNormal;
uniform vec3 portalPos;
uniform int useSlicing;

uniform float glowIntensity;

in vec3 worldPos;
in vec2 vTexCoord;

// out vec4 Outcolor;

//interpolated normal and light vector in world space
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

  for (int i = 0; i < numActiveLights.x; i++)
  {
    //vec3 light = normalize(lightDir);
    float distance = 0.0f;
    vec3 light = vec3(0.0f);
    vec3 lightColor = vec3(1.0f);
    vec3 lightIntensity = vec3(1.0f);

    // Light Attenuation default
    float constant = 1.0f;
    float linear = 0.001f;
    float quadratic = 0.000005f;

    if (i == 1)
      quadratic = 0.000005f;
    
    // Load light from list
    light = normalize(lights[i].position.xyz - worldPos); // Light source at player
    lightColor = lights[i].color.xyz;
    lightIntensity = lights[i].intensity.xyz;
    distance = length(lights[i].position.xyz - worldPos);

    float dC = max(0.0f, dot(normal, light));
    
    // Calculate Light Attenuation
    float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));
    
    // Light from all sources
    accumulatedLight += dC * lightColor * lightIntensity * attenuation;
  }
  
  vec3 lightColor = (accumulatedLight + ambient) * texColor0.xyz; 
  vec3 glowColor = glowIntensity * texColor0.xyz; 
  FragColor = vec4(lightColor + glowColor, 1.0);
  // Outcolor = vec4(dC * texColor0.xyz + ambient, 1.0);

  //to confirm texture coordinates
  // Outcolor = vec4(vTexCoord.x, vTexCoord.y, 0, 0);
  
  // fullbright
  // Outcolor = vec4(texColor0.xyz, 1.0);

  // https://learnopengl.com/Advanced-Lighting/Bloom
  float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722)); // grayscale image to find bright points
  // if(brightness > 1.0)
  if(brightness > 0.75)
      BrightColor = vec4(FragColor.rgb, 1.0);
  else
      BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

  if (useSlicing > 0)
  {
    float sliceSide = dot(portalNormal, worldPos - portalPos);
    if (sliceSide < 0.0f) discard;
  }
}