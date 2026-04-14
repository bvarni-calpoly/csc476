#version 330 core
layout (location = 0) in vec3 vertPos;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec4 debugColor;

out vec4 vertColor;

void main()
{
    gl_Position = P * V * M * vec4(vertPos, 1.0);
    vertColor = debugColor;
}