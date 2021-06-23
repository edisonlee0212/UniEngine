out vec4 FragColor;

in vec3 TexCoords;

void main()
{    
	FragColor = vec4(UE_ENVIRONMENTAL_BACKGROUND_COLOR.w == 1.0 ? UE_ENVIRONMENTAL_BACKGROUND_COLOR.xyz * UE_AMBIENT_LIGHT : texture(UE_ENVIRONMENTAL_MAP, TexCoords).xyz * UE_AMBIENT_LIGHT, 1.0);
}