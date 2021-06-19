out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec3 Tangent;
	vec2 TexCoords;
} fs_in;


void main()
{	
	vec2 texCoords = fs_in.TexCoords;
	vec4 albedo = UE_PBR_ALBEDO;
	if(UE_ALBEDO_MAP_ENABLED) albedo = vec4(texture(UE_ALBEDO_MAP, texCoords).rgb, 1.0);
	else if(UE_DIFFUSE_MAP_ENABLED) albedo = texture(UE_DIFFUSE_MAP, texCoords).rgba;
	if(UE_APLHA_DISCARD_ENABLED && albedo.a < UE_APLHA_DISCARD_OFFSET)
		discard;
	float roughness = UE_PBR_ROUGHNESS;
	float shininess = UE_PBR_SHININESS;
	float metallic = UE_PBR_METALLIC;
	vec3 normal = fs_in.Normal;
	float ao = UE_PBR_AO;
	float specular = 1.0;
	if(UE_ROUGHNESS_MAP_ENABLED) roughness = texture(UE_ROUGHNESS_MAP, texCoords).r;
	if(UE_METALLIC_MAP_ENABLED) metallic = texture(UE_METALLIC_MAP, texCoords).r;
	if(UE_AO_MAP_ENABLED) ao = texture(UE_AO_MAP, texCoords).r;
	if(UE_NORMAL_MAP_ENABLED){
		vec3 B = cross(fs_in.Normal, fs_in.Tangent);
		mat3 TBN = mat3(fs_in.Tangent, B, fs_in.Normal);
		normal = texture(UE_NORMAL_MAP, texCoords).rgb;
		normal = normal * 2.0 - 1.0;   
		normal = normalize(TBN * normal); 
	}
	
	if(UE_SPECULAR_MAP_ENABLED){
		specular = texture(UE_SPECULAR_MAP, texCoords).r;
	}
	vec3 viewDir = normalize(UE_CAMERA_POSITION - fs_in.FragPos);
	float dist = distance(fs_in.FragPos, UE_CAMERA_POSITION);
	
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo.rgb, UE_PBR_METALLIC);


	vec3 result = UE_FUNC_CALCULATE_LIGHTS(UE_ENABLE_SHADOW && UE_RECEIVE_SHADOW, shininess, albedo.rgb, 1.0, dist, normal, viewDir, fs_in.FragPos, metallic, roughness, F0);
	vec3 color = result + UE_AMBIENT_LIGHT * albedo.rgb * ao;
	FragColor = vec4(color, albedo.a);
}