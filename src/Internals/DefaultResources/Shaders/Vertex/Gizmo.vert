layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 scaleMatrix;
void main()
{
	gl_Position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * vec4(vec3(model * scaleMatrix * vec4(aPos, 1.0)), 1.0);
}