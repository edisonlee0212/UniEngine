layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 12) in mat4 inInstanceMatrix;

uniform mat4 model;
uniform vec3 scale;
void main()
{
	gl_Position = UE_CAMERA_PROJECTION_VIEW * model * inInstanceMatrix * vec4(inPosition + (inNormal / scale) * 0.05, 1.0);
}