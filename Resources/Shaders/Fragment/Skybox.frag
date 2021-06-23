out vec4 FragColor;

in vec3 TexCoords;

void main()
{    
	FragColor = vec4(UE_CAMERA_USE_CLEAR_COLOR ? texture(UE_ENVIRONMENTAL_MAP, TexCoords).xyz : UE_CAMERA_BACKGROUND_COLOR, 1.0);
}