layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 4) in vec2 aTexCoords;



out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec3 Tangent;
	vec2 TexCoords;
} vs_out;


uniform mat4 model;

void main()
{
	vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
	vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
	vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	vs_out.Normal = N;
	vs_out.Tangent = T;
	vs_out.TexCoords = aTexCoords;
	gl_Position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * vec4(vs_out.FragPos, 1.0);
}