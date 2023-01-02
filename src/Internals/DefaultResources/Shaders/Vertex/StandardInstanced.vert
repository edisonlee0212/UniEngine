layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 4) in vec2 inTexCoord;
layout (location = 12) in mat4 inInstanceMatrix;


out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec3 Tangent;
	vec2 TexCoord;
} vs_out;

uniform mat4 model;

void main()
{
	mat4 matrix = model * inInstanceMatrix;
	vs_out.FragPos = vec3(matrix * vec4(inPosition, 1.0));
	vec3 N = normalize(vec3(matrix * vec4(inNormal,    0.0)));
	vec3 T = normalize(vec3(matrix * vec4(inTangent,   0.0)));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	vs_out.Normal = N;
	vs_out.Tangent = T;
	vs_out.TexCoord = inTexCoord;    
	gl_Position =  UE_CAMERA_PROJECTION_VIEW * vec4(vs_out.FragPos, 1.0);
}