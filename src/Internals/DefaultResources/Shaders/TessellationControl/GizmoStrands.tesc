layout(vertices = 4) out;

in VS_OUT {
    vec3 FragPos;
	float Thickness;
	vec3 Normal;
} vs_in[];

out TCS_OUT {
	vec3 FragPos;
	float Thickness;
	vec3 Normal;
} tcs_out[];

void main(){
	if(gl_InvocationID == 0){
		gl_TessLevelOuter[0] = 1;
		gl_TessLevelOuter[1] = UE_STRANDS_SEGMENT_SUBDIVISION(vs_in[0].FragPos, vs_in[3].FragPos);
	}

	tcs_out[gl_InvocationID].FragPos = vs_in[gl_InvocationID].FragPos;
	tcs_out[gl_InvocationID].Thickness = vs_in[gl_InvocationID].Thickness;
	tcs_out[gl_InvocationID].Normal = vs_in[gl_InvocationID].Normal;
}