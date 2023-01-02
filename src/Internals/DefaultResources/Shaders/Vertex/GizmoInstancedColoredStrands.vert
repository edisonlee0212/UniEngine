layout (location = 0) in vec3 inPosition;
layout (location = 1) in float inThickness;
layout (location = 2) in vec3 inNormal;
//layout (location = 3) in float inTexCoords;
//layout (location = 4) in vec4 inColor;
layout (location = 11) in vec4 inColor;
layout (location = 12) in mat4 inInstanceMatrix;


out VS_OUT {
	vec3 FragPos;
	float Thickness;
	vec3 Normal;
	vec4 Color;
} vs_out;

uniform mat4 model;
uniform mat4 scaleMatrix;

void main()
{
	mat4 matrix = model * inInstanceMatrix * scaleMatrix;
	vs_out.FragPos = vec3(matrix * vec4(inPosition, 1.0));
	vs_out.Thickness = inThickness;
	vs_out.Color = inColor;
	vs_out.Normal = vec3(matrix * vec4(inNormal, 0.0));
}