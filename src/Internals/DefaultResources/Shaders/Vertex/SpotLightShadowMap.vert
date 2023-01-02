layout (location = 0) in vec3 inPosition;

uniform mat4 model;
uniform int index;
void main()
{
    gl_Position = UE_SPOT_LIGHTS[index].lightSpaceMatrix * model * vec4(inPosition, 1.0);
}