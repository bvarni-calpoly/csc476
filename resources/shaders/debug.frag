#version 330 core
uniform vec3 camPos;
uniform vec3 boxMin;
uniform vec3 boxMax;
uniform bool collided;

in vec4 vertColor;
in vec3 vertPos1;

out vec4 FragColor;

void main()
{
    bool xCollision = (camPos.x > boxMin.x) && (camPos.x < boxMax.x);
    bool yCollision = (camPos.y > boxMin.y) && (camPos.y < boxMax.y);
    bool zCollision = (camPos.z > boxMin.z) && (camPos.z < boxMax.z);

    //bool xInsideBox = (vertPos1.x > boxMin.x) && (vertPos1.x < boxMax.x);
    //bool yInsideBox = (vertPos1.y > boxMin.y) && (vertPos1.y < boxMax.y);
    //bool zInsideBox = (vertPos1.z > boxMin.z) && (vertPos1.z < boxMax.z);

    if(xCollision && yCollision && zCollision || collided)
        FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    //else if (xInsideBox)
    //    FragColor = vertColor;
    else
        FragColor = vertColor;
        //discard;
}