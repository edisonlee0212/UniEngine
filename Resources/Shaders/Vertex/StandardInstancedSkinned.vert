layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 4) in vec2 inTexCoords;

layout (location = 5) in ivec4 inBoneIds; 
layout (location = 6) in vec4 inWeights;

layout (location = 12) in mat4 inInstanceMatrix;


out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec3 Tangent;
	vec2 TexCoords;
} vs_out;

uniform mat4 model;

void main()
{
	mat4 boneTransform;
	bool boneEnabled = false;
	if(inBoneIds[0] > 0) {
		boneEnabled = true;
		boneTransform = UE_ANIM_BONES[inBoneIds[0]] * inWeights[0];
	}
    if(inBoneIds[1] > 0){
		if(!boneEnabled){
			boneEnabled = true;
			boneTransform = UE_ANIM_BONES[inBoneIds[1]] * inWeights[1];
		}else{
			boneTransform += UE_ANIM_BONES[inBoneIds[1]] * inWeights[1];
		}
	}
    if(inBoneIds[2] > 0){
		if(!boneEnabled){
			boneEnabled = true;
			boneTransform = UE_ANIM_BONES[inBoneIds[2]] * inWeights[2];
		}else{
			boneTransform += UE_ANIM_BONES[inBoneIds[2]] * inWeights[2];
		}
	}
	if(inBoneIds[3] > 0){
		if(!boneEnabled){
			boneEnabled = true;
			boneTransform = UE_ANIM_BONES[inBoneIds[3]] * inWeights[3];
		}else{
			boneTransform += UE_ANIM_BONES[inBoneIds[3]] * inWeights[3];
		}
	}

	vec4 skinnedPosition = vec4(inPos, 1.0);
	vec4 skinnedNormal = vec4(inNormal, 1.0);
	vec4 skinnedTangent = vec4(inTangent, 1.0);

	if(boneEnabled) {
		skinnedPosition = boneTransform * vec4(inPos, 1.0);
		skinnedNormal = boneTransform * vec4(inNormal, 0.0);
		skinnedTangent = boneTransform * vec4(inTangent, 0.0);
	}

	mat4 matrix = model * inInstanceMatrix;
	vs_out.FragPos = vec3(matrix * skinnedPosition);
	vec3 N = normalize(vec3(matrix * skinnedNormal));
	vec3 T = normalize(vec3(matrix * skinnedTangent));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	vs_out.Normal = N;
	vs_out.Tangent = T;
	vs_out.TexCoords = inTexCoords;    
	gl_Position =  UE_CAMERA_PROJECTION * UE_CAMERA_VIEW * vec4(vs_out.FragPos, 1.0);
}