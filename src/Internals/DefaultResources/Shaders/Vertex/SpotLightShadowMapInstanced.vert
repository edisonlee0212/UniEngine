layout (location = 0) in vec3 inPosition;
layout (location = 12) in mat4 inInstanceMatrix;

uniform mat4 model;
uniform int index;
void main()
{
    mat4 matrix = model * inInstanceMatrix;
    gl_Position = UE_SPOT_LIGHTS[index].lightSpaceMatrix * matrix * vec4(inPosition, 1.0);
}