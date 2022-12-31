layout(isolines, equal_spacing) in;

in TCS_OUT {
	vec3 FragPos;
	float Thickness;
	vec3 Normal;
	float TexCoord;
} tcs_in[];

out TES_OUT {
	vec3 FragPos;
	float Thickness;
	vec3 Normal;
	vec3 Tangent;
	float TexCoord;
} tes_out;


void hermiteInterpolation(in vec3 p0, in vec3 p1, in vec3 p2, in vec3 p3, out vec3 position, out vec3 tangent, float t)
{
	float tension = 0.8f;
	float bias = 0.0f;

	vec3 m0, m1;
	float t2,t3;
	float a0, a1, a2, a3;

	t2 = t * t;
	t3 = t2 * t;

	m0  = (p1-p0)*(1+bias)*(1-tension)/2;
	m0 += (p2-p1)*(1-bias)*(1-tension)/2;
	m1  = (p2-p1)*(1+bias)*(1-tension)/2;
	m1 += (p3-p2)*(1-bias)*(1-tension)/2;
	
	a0 =  2*t3 - 3*t2 + 1;
	a1 =    t3 - 2*t2 + t;
	a2 =    t3 -   t2;
	a3 = -2*t3 + 3*t2;

	position = vec3(a0*p1 + a1*m0 + a2*m1 + a3*p2);
	
	vec3 d1 = ((6*t2 - 6*t) * p1) + ((3*t2 - 4*t +1) * m0) + ((3*t2 - 2*t) * m1) + ((-6*t2 + 6*t) * p2);
	tangent = normalize(d1);
}

void cubicInterpolation(in vec3 v0, in vec3 v1, in vec3 v2, in vec3 v3, out vec3 position, out vec3 tangent, float t)
{
   float t2 = t * t;
   vec3 a0, a1, a2, a3;

   a0 = v3 - v2 - v0 + v1;
   a1 = v0 - v1 - a0;
   a2 = v2 - v0;
   a3 = v1;

   position = vec3(a0*t*t2 + a1*t2 + a2*t + a3);  

   vec3 d1 = vec3(3*a0*t2 + 2*a1*t + a2);
   tangent = normalize(d1);
}

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
	hermiteInterpolation(controlPoint0, controlPoint1, controlPoint2, controlPoint3, pos, tangent, gl_TessCoord.x);

	float thickness = mix(thickS, thickT, gl_TessCoord.x);
	vec3 normal = normalize(mix(vS, vT, gl_TessCoord.x));
	float newTexCoord = mix(tcs_in[0].TexCoord, tcs_in[3].TexCoord, gl_TessCoord.x);
	tes_out.TexCoord = newTexCoord; //vec2(t.x, t.y);	 
	tes_out.FragPos = pos;
	tes_out.Normal = normal;
	tes_out.Thickness = thickness;
	tes_out.Tangent = tangent;
}