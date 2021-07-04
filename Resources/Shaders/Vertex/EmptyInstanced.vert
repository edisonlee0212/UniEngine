layout (location = 0) in vec3 inPos;
layout (location = 12) in mat4 inInstanceMatrix;

uniform mat4 model;

void main()
{
	gl_Position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * model * inInstanceMatrix * vec4(inPos, 1.0);
}