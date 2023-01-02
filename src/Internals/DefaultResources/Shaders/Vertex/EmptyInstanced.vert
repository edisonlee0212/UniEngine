layout (location = 0) in vec3 inPosition;
layout (location = 12) in mat4 inInstanceMatrix;

uniform mat4 model;

void main()
{
	gl_Position = UE_CAMERA_PROJECTION_VIEW * model * inInstanceMatrix * vec4(inPosition, 1.0);
}