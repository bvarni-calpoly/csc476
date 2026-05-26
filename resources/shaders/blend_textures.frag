#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D sceneTexture;
uniform sampler2D bloomBlurTexture;
uniform float exposure;

// https://learnopengl.com/Advanced-Lighting/Bloom
void main()
{
    const float bloomIntensity = 5.0; // FIXME
    // const float gamma = 2.2;
    const float gamma = 1.0; // FIXME
    
    vec3 hdrColor = texture(sceneTexture, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlurTexture, TexCoords).rgb;

    hdrColor += bloomColor * bloomIntensity; // additive blending

    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    
    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));
    // FragColor = vec4(result, 1.0);
    FragColor = vec4(result, 1.0);
}