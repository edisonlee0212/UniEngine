layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

uniform mat4 model;
uniform vec3 scale;
void main()
{
	vec4 position = UE_CAMERA_PROJECTION_VIEW * model * vec4(inPosition + (inNormal / scale) * 0.05, 1.0);
	gl_Position = position;
}