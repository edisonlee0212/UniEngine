layout(vertices = 4) out;

in V_OUT {
    vec3 FragPos;
	float Thickness;
	vec3 Normal;
	float TexCoord;
} vs_in[];

out TCS_OUT {
	vec3 FragPos;
	float Thickness;
	vec3 Normal;
	float TexCoord;
} tcs_out[];

void main(){

	gl_TessLevelOuter[0] = 1;
	gl_TessLevelOuter[1] = min(5, UE_PIXEL_DISTANCE(vs_in[0].FragPos, vs_in[3].FragPos) * 10);

	tcs_out[gl_InvocationID].FragPos = vs_in[gl_InvocationID].FragPos;
	tcs_out[gl_InvocationID].Thickness = vs_in[gl_InvocationID].Thickness;
	tcs_out[gl_InvocationID].Normal = vs_in[gl_InvocationID].Normal;
	tcs_out[gl_InvocationID].TexCoord = vs_in[gl_InvocationID].TexCoord;
}