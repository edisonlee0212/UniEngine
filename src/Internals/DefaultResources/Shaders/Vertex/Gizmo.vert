layout (location = 0) in vec3 inPosition;

uniform mat4 model;
uniform mat4 scaleMatrix;
void main()
{
	gl_Position = UE_CAMERA_PROJECTION_VIEW * vec4(vec3(model * scaleMatrix * vec4(inPosition, 1.0)), 1.0);
}