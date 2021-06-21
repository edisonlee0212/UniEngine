layout (location = 0) out vec4 gPositionShadow;
layout (location = 1) out vec4 gNormalShininess;
layout (location = 2) out vec4 gAlbedoSpecular;
layout (location = 3) out vec4 gMetallicRoughnessAO;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec3 Tangent;
	vec2 TexCoords;
} fs_in;

void main()
{
	vec3 B = cross(fs_in.Normal, fs_in.Tangent);
	mat3 TBN = mat3(fs_in.Tangent, B, fs_in.Normal);

	vec2 texCoords = fs_in.TexCoords;
	float depth = 0.0;

	vec3 normal = fs_in.Normal;
	if(UE_NORMAL_MAP_ENABLED){
		normal = texture(UE_NORMAL_MAP, texCoords).rgb;
		normal = normal * 2.0 - 1.0;   
		normal = normalize(TBN * normal); 
	}

	vec4 albedo = UE_PBR_ALBEDO;
	float roughness = UE_PBR_ROUGHNESS;
	float metallic = UE_PBR_METALLIC;
	float ao = UE_PBR_AO;
	if(UE_ALBEDO_MAP_ENABLED) albedo = vec4(texture(UE_ALBEDO_MAP, texCoords).rgb, 1.0);

	if(UE_APLHA_DISCARD_ENABLED && albedo.a < UE_APLHA_DISCARD_OFFSET)
		discard;

	if(UE_ROUGHNESS_MAP_ENABLED) roughness = texture(UE_ROUGHNESS_MAP, texCoords).r;
	if(UE_METALLIC_MAP_ENABLED) metallic = texture(UE_METALLIC_MAP, texCoords).r;
	//if(UE_AO_MAP_ENABLED) ao = texture(UE_AO_MAP, texCoords).r;


	// store the fragment position vector in the first gbuffer texture
	gPositionShadow.rgb = fs_in.FragPos - (depth * fs_in.Normal);
	gPositionShadow.a = float(UE_ENABLE_SHADOW && UE_RECEIVE_SHADOW);

	// also store the per-fragment normals into the gbuffer
	gNormalShininess.rgb = (gl_FrontFacing ? 1.0 : -1.0) * normalize(normal);
	
	// store specular intensity in gAlbedoSpecular's alpha component
	float specular = 1.0;
	
	gAlbedoSpecular = vec4(albedo.rgb, specular);
	gMetallicRoughnessAO = vec4(metallic, roughness, ao, 1.0);
}