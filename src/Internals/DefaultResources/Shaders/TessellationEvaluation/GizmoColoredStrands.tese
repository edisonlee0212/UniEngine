layout(isolines, equal_spacing) in;

in TCS_OUT {
	vec3 FragPos;
	float Thickness;
	vec3 Normal;
	vec4 Color;
} tcs_in[];

out TES_OUT {
	vec3 FragPos;
	float Thickness;
	vec3 Normal;
	vec3 Tangent;
	vec4 Color;
} tes_out;




void main(){
	vec3 position, normal, tangent, tempV;
	float thickness, tempF;
	vec4 color, tempV2;
	UE_SPLINE_INTERPOLATION(tcs_in[0].FragPos, tcs_in[1].FragPos, tcs_in[2].FragPos, tcs_in[3].FragPos, position, tangent, gl_TessCoord.x);
	UE_SPLINE_INTERPOLATION(tcs_in[0].Normal, tcs_in[1].Normal, tcs_in[2].Normal, tcs_in[3].Normal, normal, tempV, gl_TessCoord.x);
	UE_SPLINE_INTERPOLATION(tcs_in[0].Color, tcs_in[1].Color, tcs_in[2].Color, tcs_in[3].Color, color, tempV2, gl_TessCoord.x);
	UE_SPLINE_INTERPOLATION(tcs_in[0].Thickness, tcs_in[1].Thickness, tcs_in[2].Thickness, tcs_in[3].Thickness, thickness, tempF, gl_TessCoord.x);


	tes_out.FragPos = position;
	tes_out.Normal = normal;
	tes_out.Thickness = thickness;
	tes_out.Tangent = tangent;
	tes_out.Color = color;
}