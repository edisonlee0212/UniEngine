out vec4 FragColor;

in vec3 TexCoords;

void main()
{    
	FragColor = vec4(UE_CAMERA_SKYBOX_ENABLED ? texture(UE_CAMERA_SKYBOX, TexCoords).xyz : UE_CAMERA_BACKGROUND_COLOR, 1.0);
}