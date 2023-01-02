layout (location = 0) in vec3 inPosition;

uniform mat4 model;

void main()
{
    gl_Position = model * vec4(inPosition, 1.0);
}