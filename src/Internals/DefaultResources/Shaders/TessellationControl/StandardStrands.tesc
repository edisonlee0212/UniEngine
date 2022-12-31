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
	vec3 vSWorld = vs_in[0].FragPos; 
	vec3 vTWorld = vs_in[3].FragPos;

	float dS = length(UE_CAMERA_POSITION - vSWorld);
	float dT = length(UE_CAMERA_POSITION - vTWorld);

	float dist = max(1, min(dS, dT));
	float r = vs_in[0].Thickness;
	float p = 1;

	if(dist <= 0.01)
		dist = 0.01;

	if(log((r/dist)+1) > 0.00035 || dist < 15)
	{
		float l = length(vs_in[0].FragPos - vs_in[3].FragPos);
		float d = length(vs_in[0].FragPos - UE_CAMERA_POSITION);
		float c = 100;
		float t = c * l/d;
		p = max(1, min(3, t));
		
	}

	gl_TessLevelOuter[0] = 1;
	gl_TessLevelOuter[1] = p;

	tcs_out[gl_InvocationID].FragPos = vs_in[gl_InvocationID].FragPos;
	tcs_out[gl_InvocationID].Thickness = vs_in[gl_InvocationID].Thickness;
	tcs_out[gl_InvocationID].Normal = vs_in[gl_InvocationID].Normal;
	tcs_out[gl_InvocationID].TexCoord = vs_in[gl_InvocationID].TexCoord;
}