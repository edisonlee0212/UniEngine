layout (location = 0) in vec3 inPos;
layout (location = 1) in float inThickness;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in float inTexCoord;
//layout (location = 4) in vec4 inColor;


out V_OUT {
	vec3 FragPos;
	float Thickness;
	vec3 Normal;
	float TexCoord;
} vs_out;

uniform mat4 model;

void main()
{
	vs_out.FragPos = vec3(model * vec4(inPos, 1.0));
	vs_out.Thickness = inThickness;
	vs_out.TexCoord = inTexCoord;
	vs_out.Normal = vec3(model * vec4(inNormal, 0.0));
}