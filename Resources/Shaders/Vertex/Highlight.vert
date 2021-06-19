layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform vec3 scale;
void main()
{
	vec4 position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * model * vec4(aPos + (aNormal / scale) * 0.05, 1.0);
	gl_Position = position;
}