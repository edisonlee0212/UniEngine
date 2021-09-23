out vec4 FragColor;

in vec3 TexCoords;

void main()
{
	vec3 envColor = UE_ENVIRONMENTAL_BACKGROUND_COLOR.w == 1.0 ? UE_ENVIRONMENTAL_BACKGROUND_COLOR.xyz * UE_ENVIRONMENTAL_LIGHTING_INTENSITY : pow(texture(UE_ENVIRONMENTAL_MAP, TexCoords).rgb, vec3(1.0 / UE_ENVIRONMENTAL_MAP_GAMMA)) * UE_ENVIRONMENTAL_LIGHTING_INTENSITY;

	envColor = pow(envColor, vec3(1.0 / UE_GAMMA));

	FragColor = vec4(envColor, 1.0);
}