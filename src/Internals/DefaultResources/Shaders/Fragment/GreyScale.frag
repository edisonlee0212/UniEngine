layout (location = 0) out float FragColor;

in VS_OUT {
	vec2 TexCoords;
} fs_in;

uniform sampler2D inputTex;

void main()
{
	vec3 color = texture(inputTex, fs_in.TexCoords).rgb;
	FragColor = (color.x + color.y + color.z) / 3.0;
}