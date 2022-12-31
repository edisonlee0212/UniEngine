layout (location = 0) in vec3 inPos;
layout (location = 12) in mat4 inInstanceMatrix;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    mat4 matrix = model * inInstanceMatrix;
    gl_Position = matrix * vec4(inPos, 1.0);
}