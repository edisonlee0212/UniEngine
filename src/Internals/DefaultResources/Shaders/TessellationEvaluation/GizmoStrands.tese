layout(isolines, equal_spacing) in;

in TCS_OUT {
	vec3 FragPos;
	float Thickness;
	vec3 Normal;
} tcs_in[];

out TES_OUT {
	vec3 FragPos;
	float Thickness;
	vec3 Normal;
	vec3 Tangent;
} tes_out;

void main(){
	//vec3 t = vec3(gl_TessCoord.x, gl_TessCoord.y, gl_TessCoord.z);
			
	vec3 vS = tcs_in[0].Normal; // V of PTF
	vec3 vT = tcs_in[3].Normal;

	float thickS = tcs_in[0].Thickness; // Thickness
	float thickT = tcs_in[3].Thickness;    

	vec3 controlPoint0 = tcs_in[0].FragPos;
	vec3 controlPoint1 = tcs_in[1].FragPos;
	vec3 controlPoint2 = tcs_in[2].FragPos;
	vec3 controlPoint3 = tcs_in[3].FragPos;

	vec3 pos, tangent;
	//cubicInterpolation(p_minus_1, pi, p_plus_1, p_plus_2, pos, tan, gl_TessCoord.x);
	UE_HERMITE_INTERPOLATION(controlPoint0, controlPoint1, controlPoint2, controlPoint3, pos, tangent, gl_TessCoord.x);

	float thickness = mix(thickS, thickT, gl_TessCoord.x);
	vec3 normal = normalize(mix(vS, vT, gl_TessCoord.x));
	//tangent = normalize(mix(tS.xyz, tT.xyz, gl_TessCoord.x));
	//float newTexCoord = mix(tcs_in[0].TexCoord, tcs_in[3].TexCoord, gl_TessCoord.x);
	//tes_out.TexCoord = newTexCoord; //vec2(t.x, t.y);	 
	tes_out.FragPos = pos;
	tes_out.Normal = normal;
	tes_out.Thickness = thickness;
	tes_out.Tangent = tangent;
}