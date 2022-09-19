layout (location = 0) in vec3 aPos;
layout (location = 3) in vec3 aColor;

uniform mat4 model;
uniform mat4 scaleMatrix;

out VS_OUT {
	vec4 surfaceColor;
} vs_out;

void main()
{
    vs_out.surfaceColor = vec4(aColor, 1.0);
	gl_Position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * vec4(vec3(model * scaleMatrix * vec4(aPos, 1.0)), 1.0);
}