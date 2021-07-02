layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 4) in vec2 inTexCoords;

layout (location = 5) in ivec4 inBoneIds; 
layout (location = 6) in vec4 inWeights;

const int MAX_BONES = 100;
uniform mat4 bones[MAX_BONES];

out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec3 Tangent;
	vec2 TexCoords;
} vs_out;


uniform mat4 model;

void main()
{
	mat4 boneTransform = bones[inBoneIds[0]] * inWeights[0];
    boneTransform += bones[inBoneIds[1]] * inWeights[1];
    boneTransform += bones[inBoneIds[2]] * inWeights[2];
    boneTransform += bones[inBoneIds[3]] * inWeights[3];
	vec4 skinnedPosition = boneTransform * vec4(inPos, 1.0);
	vec4 skinnedNormal = boneTransform * vec4(inNormal, 1.0);
	vec4 skinnedTangent = boneTransform * vec4(inTangent, 1.0);

	vs_out.FragPos = vec3(model * skinnedPosition);
	vec3 N = normalize(vec3(model * skinnedNormal));
	vec3 T = normalize(vec3(model * skinnedTangent));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	vs_out.Normal = N;
	vs_out.Tangent = T;
	vs_out.TexCoords = inTexCoords;
	gl_Position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * skinnedPosition;
}