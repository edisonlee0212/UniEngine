out vec4 FragColor;

in vec3 TexCoords;

void main()
{
	vec3 envColor = UE_ENVIRONMENTAL_BACKGROUND_COLOR.w == 1.0 ? UE_ENVIRONMENTAL_BACKGROUND_COLOR.xyz * UE_AMBIENT_LIGHT : texture(UE_ENVIRONMENTAL_MAP, TexCoords).xyz * UE_AMBIENT_LIGHT;
	// HDR tonemap and gamma correct
    //envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    

	FragColor = vec4(envColor, 1.0);
}