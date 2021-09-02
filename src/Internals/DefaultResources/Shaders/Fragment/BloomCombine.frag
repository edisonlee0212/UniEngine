layout (location = 0) out vec4 FragColor;
in VS_OUT {
	vec2 TexCoords;
} fs_in;

uniform sampler2D flatColor;
uniform sampler2D brightColor;
void main()
{
	
	vec3 result = texture(flatColor, fs_in.TexCoords).rgb + texture(brightColor, fs_in.TexCoords).rgb;
	FragColor = vec4(result, 1.0);
}