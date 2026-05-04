https://learnopengl.com/Advanced-OpenGL/Geometry-Shader

#version 330 core
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNor;

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 V;
uniform mat4 M;

void main()
{
    gl_Position = P * V * M * vec4(vertPos, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(V * M)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix * vertNor, 0.0)));
}