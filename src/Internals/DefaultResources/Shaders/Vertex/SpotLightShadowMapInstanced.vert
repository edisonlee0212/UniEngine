layout (location = 0) in vec3 aPos;
layout (location = 12) in mat4 aInstanceMatrix;

uniform mat4 model;
uniform int index;
void main()
{
    mat4 matrix = model * aInstanceMatrix;
    gl_Position = UE_SPOT_LIGHTS[index].lightSpaceMatrix * matrix * vec4(aPos, 1.0);
}