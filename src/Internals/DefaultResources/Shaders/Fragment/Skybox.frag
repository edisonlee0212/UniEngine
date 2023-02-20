out vec4 FragColor;

in vec3 TexCoord;

void main()
{
	vec3 envColor = UE_CAMERA_CLEAR_COLOR.w == 1.0 ? UE_CAMERA_CLEAR_COLOR.xyz * UE_BACKGROUND_INTENSITY : pow(texture(UE_SKYBOX, TexCoord).rgb, vec3(1.0 / UE_ENVIRONMENTAL_MAP_GAMMA)) * UE_BACKGROUND_INTENSITY;

	envColor = pow(envColor, vec3(1.0 / UE_GAMMA));

	FragColor = vec4(envColor, 1.0);
}