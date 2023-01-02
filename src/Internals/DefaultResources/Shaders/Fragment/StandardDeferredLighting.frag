out vec4 FragColor;

in VS_OUT {
	vec2 TexCoord;
} fs_in;

uniform sampler2D gDepth;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMetallicRoughnessEmissionAmbient;

void main()
{
	vec3 normal = 		texture(gNormal, fs_in.TexCoord).rgb;
	float ndcDepth = 	texture(gDepth, fs_in.TexCoord).r;
	float depth = UE_LINEARIZE_DEPTH(ndcDepth);

	float metallic = 	texture(gMetallicRoughnessEmissionAmbient, fs_in.TexCoord).r;
	float roughness = 	texture(gMetallicRoughnessEmissionAmbient, fs_in.TexCoord).g;
	float emission = 	texture(gMetallicRoughnessEmissionAmbient, fs_in.TexCoord).b;
	float ao = 			texture(gMetallicRoughnessEmissionAmbient, fs_in.TexCoord).a;

	vec3 albedo = 		texture(gAlbedo, fs_in.TexCoord).rgb;

	vec3 fragPos = UE_DEPTH_TO_WORLD_POS(fs_in.TexCoord, ndcDepth);

	vec3 cameraPosition = UE_CAMERA_POSITION();
	vec3 viewDir = normalize(cameraPosition - fragPos);
	bool receiveShadow = true;
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);
	vec3 result = UE_FUNC_CALCULATE_LIGHTS(receiveShadow, albedo, 1.0, depth, normal, viewDir, fragPos, metallic, roughness, F0);
	vec3 ambient = UE_FUNC_CALCULATE_ENVIRONMENTAL_LIGHT(albedo, normal, viewDir, metallic, roughness, F0);
	vec3 color = result + emission * normalize(albedo.xyz) + ambient * ao;
	color = pow(color, vec3(1.0 / UE_GAMMA));
	int width = textureSize(gAlbedo, 0).x;
	FragColor = vec4(color, 1.0);
}