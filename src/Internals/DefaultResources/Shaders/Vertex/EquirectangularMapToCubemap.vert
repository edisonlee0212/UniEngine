layout (location = 0) in vec3 inPosition;

out vec3 WorldPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    WorldPos = inPosition;
    gl_Position =  projection * view * vec4(inPosition, 1.0);
}