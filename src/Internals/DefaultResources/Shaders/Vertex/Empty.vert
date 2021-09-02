layout (location = 0) in vec3 inPos;

uniform mat4 model;

void main()
{
	gl_Position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * model * vec4(inPos, 1.0);
}