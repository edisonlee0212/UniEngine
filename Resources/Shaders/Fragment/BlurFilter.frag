layout (location = 0) out vec4 FragColor;

in VS_OUT {
	vec2 TexCoords;
} fs_in;

uniform sampler2D image;
uniform vec4 bezier;
uniform float sampleScale;
uniform float intensity;
uniform int diffusion;
uniform bool horizontal;
uniform float clamp;

float Bezier(float dt){
	float t = 1.0 - dt;
	float t1 = 1.0 - t;
	return t * (3.0 * t1 * t1 * (bezier.xy + bezier.zw) + t * t * vec2(1, 1)).y;
}


void main()
{
	vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
	vec3 result = texture(image, fs_in.TexCoords).rgb * Bezier(0) * intensity;
	if(horizontal)
	{
		for(int i = 1; i < diffusion + 1; ++i)
		{
			result += texture(image, fs_in.TexCoords + sampleScale * vec2(tex_offset.x * i, 0.0)).rgb * Bezier(float(i) / diffusion) * intensity;
			result += texture(image, fs_in.TexCoords - sampleScale * vec2(tex_offset.x * i, 0.0)).rgb * Bezier(float(i) / diffusion) * intensity;
		}
	}
	else
	{
		for(int i = 1; i < diffusion + 1; ++i)
		{
			result += texture(image, fs_in.TexCoords + sampleScale * vec2(0.0, tex_offset.y * i)).rgb * Bezier(float(i) / diffusion) * intensity;
			result += texture(image, fs_in.TexCoords - sampleScale * vec2(0.0, tex_offset.y * i)).rgb * Bezier(float(i) / diffusion) * intensity;
		}
	}
	FragColor = vec4(result, 1.0);
}