layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

uniform mat4 model;
uniform mat4 scaleMatrix;

out VS_OUT {
	vec4 Color;
} vs_out;

void main()
{
    vs_out.Color = vec4(inNormal, 1.0);
	gl_Position = UE_CAMERA_PROJECTION_VIEW * vec4(vec3(model * scaleMatrix * vec4(inPosition, 1.0)), 1.0);
}