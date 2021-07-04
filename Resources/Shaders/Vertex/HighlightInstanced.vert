layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 12) in mat4 inInstanceMatrix;

uniform mat4 model;
uniform vec3 scale;
void main()
{
	vec4 position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * model * inInstanceMatrix * vec4(inPos + (inNormal / scale) * 0.05, 1.0);
	gl_Position = position;
}