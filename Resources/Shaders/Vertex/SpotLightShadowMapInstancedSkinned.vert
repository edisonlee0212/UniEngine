layout (location = 0) in vec3 aPos;
layout (location = 5) in ivec4 inBoneIds; 
layout (location = 6) in vec4 inWeights;
layout (location = 7) in ivec4 inBoneIds2; 
layout (location = 8) in vec4 inWeights2;
layout (location = 12) in mat4 aInstanceMatrix;

uniform int index;
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
    gl_Position = UE_SPOT_LIGHTS[index].lightSpaceMatrix * boneTransform * aInstanceMatrix * vec4(aPos, 1.0);
}