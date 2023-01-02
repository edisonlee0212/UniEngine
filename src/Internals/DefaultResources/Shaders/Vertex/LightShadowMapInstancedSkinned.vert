layout (location = 0) in vec3 inPosition;
layout (location = 5) in ivec4 inBoneIds; 
layout (location = 6) in vec4 inWeights;
layout (location = 7) in ivec4 inBoneIds2; 
layout (location = 8) in vec4 inWeights2;

layout (location = 12) in mat4 inInstanceMatrix;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;
void main()
{
	mat4 boneTransform = UE_ANIM_BONES[inBoneIds[0]] * inWeights[0];
    if(inBoneIds[1] != -1){
		boneTransform += UE_ANIM_BONES[inBoneIds[1]] * inWeights[1];
	}
    if(inBoneIds[2] != -1){
		boneTransform += UE_ANIM_BONES[inBoneIds[2]] * inWeights[2];
	}
	if(inBoneIds[3] != -1){
		boneTransform += UE_ANIM_BONES[inBoneIds[3]] * inWeights[3];
	}
	if(inBoneIds2[0] != -1){
		boneTransform += UE_ANIM_BONES[inBoneIds2[0]] * inWeights2[0];
	}
    if(inBoneIds2[1] != -1){
		boneTransform += UE_ANIM_BONES[inBoneIds2[1]] * inWeights2[1];
	}
	if(inBoneIds2[2] != -1){
		boneTransform += UE_ANIM_BONES[inBoneIds2[2]] * inWeights2[2];
	}
	if(inBoneIds2[3] != -1){
		boneTransform += UE_ANIM_BONES[inBoneIds2[3]] * inWeights2[3];
	}

    mat4 matrix = model * boneTransform * inInstanceMatrix;
    gl_Position = matrix * vec4(inPosition, 1.0);
}