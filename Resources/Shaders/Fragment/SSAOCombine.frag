layout (location = 0) out vec3 FragColor;
in VS_OUT {
	vec2 TexCoords;
} fs_in;

uniform sampler2D originalColor;
uniform sampler2D ao;
void main()
{
	FragColor = texture(originalColor, fs_in.TexCoords).rgb * (1.0 - texture(ao, fs_in.TexCoords).r);
}