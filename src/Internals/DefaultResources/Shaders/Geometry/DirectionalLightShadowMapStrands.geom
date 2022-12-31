layout(lines, invocations = 1) in;
layout(triangle_strip, max_vertices = 24) out; // (1 * 4 + 2) * 4

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

uniform int index;
uniform mat4 model;
const float PI2 = 6.28318531;
const float c = 1000.0;

void main(){
	for(int split = 0; split < 4; ++split)
    {
		mat4 splitMatrix = UE_DIRECTIONAL_LIGHTS[index].lightSpaceMatrix[split];
        gl_Layer = split; // built-in variable that specifies to which face we render.
		for(int i = 0; i < gl_VerticesIn - 1; ++i)
		{
			//Reading Data
			vec3 posS = vec3(inverse(model) * vec4(tes_in[i].FragPos, 1.0));
			vec3 posT = vec3(inverse(model) * vec4(tes_in[i + 1].FragPos, 1.0));

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

			int pS = 2 * int(c * rS);
			int pT = 2 * int(c * rT);
			
			int forMax = 3;

			for(int k = 0; k <= forMax; k += 1)
			{
				int tempIS = int(k * pS / forMax);
				float angleS = (PI2 / pS) * tempIS;
								 
				int tempIT = int(k * pT/forMax);
				float angleT = (PI2 / pT) * tempIT;

				vec3 newPS = vec3(model * vec4(posS.xyz + (v11 * sin(-angleS) + v12 * cos(-angleS)) * rS, 1.0));
				vec3 newPT = vec3(model * vec4(posT.xyz + (v21 * sin(-angleT) + v22 * cos(-angleT)) * rT, 1.0));

				gl_Position = splitMatrix * vec4(newPS, 1);
				EmitVertex();

				gl_Position = splitMatrix * vec4(newPT, 1);
				EmitVertex();
			}
		}

		EndPrimitive();
	}
}