layout(lines, invocations = 1) in;
layout(triangle_strip, max_vertices = 66) out;

in TES_OUT {
	vec3 FragPos;
	float Thickness;
	vec3 Normal;
	vec3 Tangent;
} tes_in[];

//out GS_OUT {
//	vec3 FragPos;
//	vec3 Normal;
//	vec3 Tangent;
//	vec2 TexCoords;
//} gs_out;

const float PI2 = 6.28318531;
const float c = 1000.0;

void main(){
	mat4 cameraProjectionView = UE_CAMERA_PROJECTION * UE_CAMERA_VIEW;

	for(int i = 0; i < gl_VerticesIn - 1; ++i)
	{
		//Reading Data
		vec3 posS = tes_in[i].FragPos;
		vec3 posT = tes_in[i + 1].FragPos;

		vec3 vS = tes_in[i].Normal;
		vec3 vT = tes_in[i + 1].Normal;
		
		vec3 tS = tes_in[i].Tangent;
		vec3 tT = tes_in[i + 1].Tangent;

		float thickS = tes_in[i].Thickness;
		float thickT = tes_in[i + 1].Thickness;
			  
		
		//Computing
		vec3 v11 = normalize(vS);        
		vec3 v12 = normalize(cross(vS, tS));
	 
		vec3 v21 = normalize(vT);
		vec3 v22 = normalize(cross(vT, tT)); 

		float rS = max(0.001, thickS); 
		float rT = max(0.001, thickT);

		float dS = max(1, length(UE_CAMERA_POSITION - posS));
		float dT = max(1, length(UE_CAMERA_POSITION - posT));
 
		int pS = 2 * int(c * rS / dS);
		int pT = 2 * int(c * rT / dT);

		//pT = 3;
		//pS = pT;

		pS = max(3, min(15, pS));
		pT = max(3, min(15, pT));

		int forMax = max(pS, pT);

		for(int k = 0; k <= forMax; k += 1)
		{
			int tempIS = int(k * pS / forMax);
			float angleS = (PI2 / pS) * tempIS;
								 
			int tempIT = int(k * pT/forMax);
			float angleT = (PI2 / pT) * tempIT;

			vec3 newPS = posS.xyz + (v11 * sin(angleS) + v12 * cos(angleS)) * rS;
			vec3 newPT = posT.xyz + (v21 * sin(angleT) + v22 * cos(angleT)) * rT;

			//Source Vertex
			//vec3 normal = normalize(posS - newPS);
			//gs_out.FragPos = tes_in[i].FragPos;
			//gs_out.Normal = normal;
			//gs_out.Tangent = tes_in[i].Tangent;
			//gs_out.TexCoord = vec2(1.0 * tempIS / pS, tes_in[i].TexCoord);

			gl_Position = cameraProjectionView * vec4(newPS, 1);
			EmitVertex();

			//Target Vertex
			//normal = normalize(posT.xyz - newPT);
			//gs_out.FragPos = tes_in[i + 1].FragPos;
			//gs_out.Normal = normal;
			//gs_out.Tangent = tes_in[i + 1].Tangent;
			//gs_out.TexCoord = vec2(1.0 * tempIT / pT, tes_in[i + 1].TexCoord);

			gl_Position = cameraProjectionView * vec4(newPT, 1);
			EmitVertex();
		}
	}

	EndPrimitive();
}