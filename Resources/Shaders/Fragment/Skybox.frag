out vec4 FragColor;

in vec3 TexCoords;

void main()
{
	vec3 envColor = UE_ENVIRONMENTAL_BACKGROUND_COLOR.w == 1.0 ? UE_ENVIRONMENTAL_BACKGROUND_COLOR.xyz * UE_AMBIENT_LIGHT : pow(texture(UE_ENVIRONMENTAL_MAP, TexCoords).rgb, vec3(1.0 / UE_ENVIRONMENTAL_MAP_GAMMA)) * UE_AMBIENT_LIGHT;
	FragColor = vec4(envColor, 1.0);
}