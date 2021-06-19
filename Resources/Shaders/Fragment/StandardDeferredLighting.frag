out vec4 FragColor;

in VS_OUT {
	vec2 TexCoords;
} fs_in;

uniform sampler2D gPositionShadow;
uniform sampler2D gNormalShininess;
uniform sampler2D gAlbedoSpecular;
uniform sampler2D gMetallicRoughnessAO;

void main()
{	
	vec3 fragPos = texture(gPositionShadow, fs_in.TexCoords).rgb;

	vec3 albedo = texture(gAlbedoSpecular, fs_in.TexCoords).rgb;
	vec3 normal = texture(gNormalShininess, fs_in.TexCoords).rgb;
	float metallic = texture(gMetallicRoughnessAO, fs_in.TexCoords).r;
	float roughness = texture(gMetallicRoughnessAO, fs_in.TexCoords).g;
	float ao = texture(gMetallicRoughnessAO, fs_in.TexCoords).b;
	float shininess = texture(gNormalShininess, fs_in.TexCoords).a;
	float specular = texture(gAlbedoSpecular, fs_in.TexCoords).a;
	bool receiveShadow = bool(texture(gPositionShadow, fs_in.TexCoords).a);

	vec3 viewDir = normalize(UE_CAMERA_POSITION - fragPos);
	float dist = distance(fragPos, UE_CAMERA_POSITION);

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);

	vec3 result = UE_FUNC_CALCULATE_LIGHTS(receiveShadow, shininess, albedo, 1.0, dist, normal, viewDir, fragPos, metallic, roughness, F0);
	vec3 color = result + UE_AMBIENT_LIGHT * albedo * ao;

	//float gamma = 2.2;
	// exposure tone mapping
    //vec3 mapped = vec3(1.0) - exp(-color * 1.0);
    // gamma correction 
    //mapped = pow(mapped, vec3(1.0 / gamma));

	FragColor = vec4(color, 1.0);
}