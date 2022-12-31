layout (location = 0) in vec3 inPos;

uniform mat4 model;
uniform mat4 scaleMatrix;
void main()
{
	gl_Position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * vec4(vec3(model * scaleMatrix * vec4(inPos, 1.0)), 1.0);
}