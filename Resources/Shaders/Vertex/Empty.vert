layout (location = 0) in vec3 aPos;

uniform mat4 model;

void main()
{
	gl_Position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * model * vec4(aPos, 1.0);
}