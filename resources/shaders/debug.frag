#version 330 core

uniform vec3 camPos;
uniform vec3 boxMin;
uniform vec3 boxMax;
uniform int collided;

in vec4 vertColor;

out vec4 FragColor;

void main()
{
    bool xCollision = (camPos.x > boxMin.x) && (camPos.x < boxMax.x);
    bool yCollision = (camPos.y > boxMin.y) && (camPos.y < boxMax.y);
    bool zCollision = (camPos.z > boxMin.z) && (camPos.z < boxMax.z);

    if(xCollision && yCollision && zCollision || collided == 1)
        FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    else
        FragColor = vertColor;
        //discard;
}