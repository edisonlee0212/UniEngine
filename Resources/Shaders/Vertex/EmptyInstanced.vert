layout (location = 0) in vec3 aPos;
layout (location = 12) in mat4 aInstanceMatrix;

uniform mat4 model;

void main()
{
	gl_Position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * model * aInstanceMatrix * vec4(aPos, 1.0);
}