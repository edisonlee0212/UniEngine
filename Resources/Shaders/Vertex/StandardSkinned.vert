layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 4) in vec2 inTexCoords;

layout (location = 5) in ivec4 inBoneIds; 
layout (location = 6) in vec4 inWeights;

out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec3 Tangent;
	vec2 TexCoords;
} vs_out;


uniform mat4 model;

void main()
{
	mat4 boneTransform = UE_ANIM_BONES[inBoneIds[0]] * inWeights[0];
    if(inBoneIds[1] > 0){
		boneTransform += UE_ANIM_BONES[inBoneIds[1]] * inWeights[1];
	}
    if(inBoneIds[2] > 0){
		boneTransform += UE_ANIM_BONES[inBoneIds[2]] * inWeights[2];
	}
	if(inBoneIds[3] > 0){
		boneTransform += UE_ANIM_BONES[inBoneIds[3]] * inWeights[3];
	}

	vs_out.FragPos = vec3(model * boneTransform * vec4(inPos, 1.0));
	vec3 N = normalize(vec3(model * boneTransform * vec4(inNormal, 0.0)));
	vec3 T = normalize(vec3(model * boneTransform * vec4(inTangent, 0.0)));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	vs_out.Normal = N;
	vs_out.Tangent = T;
	vs_out.TexCoords = inTexCoords;
	gl_Position = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * vec4(vs_out.FragPos, 1.0);
}