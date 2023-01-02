layout (location = 0) in vec3 inPosition;
layout (location = 12) in mat4 inInstanceMatrix;
uniform mat4 model;
uniform mat4 scaleMatrix;
void main()
{
	mat4 matrix = model * inInstanceMatrix * scaleMatrix;
	gl_Position = UE_CAMERA_PROJECTION_VIEW * vec4(vec3(matrix * vec4(inPosition, 1.0)), 1.0);
}