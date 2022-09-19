layout (location = 0) in vec3 aPos;
layout (location = 11) in vec4 instanceColor;
layout (location = 12) in mat4 instanceMatrix;
uniform mat4 model;
uniform mat4 scaleMatrix;
out VS_OUT {
	vec4 surfaceColor;
} vs_out;

void main()
{
	mat4 matrix = model * instanceMatrix * scaleMatrix;
	vs_out.surfaceColor = instanceColor;
	gl_Position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * vec4(vec3(matrix * vec4(aPos, 1.0)), 1.0);
}