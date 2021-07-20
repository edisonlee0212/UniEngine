out vec4 FragColor;

in VS_OUT {
	vec2 TexCoords;
} fs_in;

uniform sampler2D gDepth;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoEmission;
uniform sampler2D gMetallicRoughnessAmbient;



void main()
{
	vec3 normal = texture(gNormal, fs_in.TexCoords).rgb;
	float ndcDepth = texture(gDepth, fs_in.TexCoords).r;
	float depth = UE_LINEARIZE_DEPTH(ndcDepth);
	float metallic = texture(gMetallicRoughnessAmbient, fs_in.TexCoords).r;
	float roughness = texture(gMetallicRoughnessAmbient, fs_in.TexCoords).g;
	float ao = texture(gMetallicRoughnessAmbient, fs_in.TexCoords).b;
	vec3 albedo = texture(gAlbedoEmission, fs_in.TexCoords).rgb;
	float emission = texture(gAlbedoEmission, fs_in.TexCoords).a;
	vec3 fragPos = UE_DEPTH_TO_WORLD_POS(fs_in.TexCoords, ndcDepth);
	vec3 viewDir = normalize(UE_CAMERA_POSITION - fragPos);
	bool receiveShadow = true;//bool(texture(gMaterialProps, fs_in.TexCoords).a);
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);
	vec3 result = UE_FUNC_CALCULATE_LIGHTS(receiveShadow, albedo, 1.0, depth, normal, viewDir, fragPos, metallic, roughness, F0);
	vec3 ambient = UE_FUNC_CALCULATE_ENVIRONMENTAL_LIGHT(albedo, normal, viewDir, metallic, roughness, F0);
	vec3 color = result + emission * normalize(albedo.xyz) + ambient * ao * UE_AMBIENT_LIGHT;
	color = pow(color, vec3(1.0 / UE_GAMMA));
	int width = textureSize(gAlbedoEmission, 0).x;
	FragColor = vec4(color, 1.0);
}