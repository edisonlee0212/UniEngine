layout (location = 0) out vec4 FragColor;

in VS_OUT {
	vec2 TexCoords;
} fs_in;

uniform sampler2D inputTex;

void main()
{
	float color = texture(inputTex, fs_in.TexCoords).r;
	FragColor = vec4(color, color, color, 1.0);
}