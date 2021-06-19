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


vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);
void main()
{
	vec3 B = cross(fs_in.Normal, fs_in.Tangent);
	mat3 TBN = mat3(fs_in.Tangent, B, fs_in.Normal);

	vec2 texCoords = fs_in.TexCoords;
	float depth = 0.0;

	if(UE_DISPLACEMENT_MAP_ENABLED){
		vec3 viewDir = reflect(normalize(UE_CAMERA_POSITION - fs_in.FragPos), fs_in.Normal);
		vec2 result = ParallaxMapping(texCoords, normalize(TBN * viewDir));
		texCoords = result;
	}

	vec3 normal = fs_in.Normal;
	if(UE_NORMAL_MAP_ENABLED){
		normal = texture(UE_NORMAL_MAP, texCoords).rgb;
		normal = normal * 2.0 - 1.0;   
		normal = normalize(TBN * normal); 
	}

	vec4 albedo = UE_PBR_ALBEDO;
	float roughness = UE_PBR_ROUGHNESS;
	gNormalShininess.a = UE_PBR_SHININESS;
	float metallic = UE_PBR_METALLIC;
	float ao = UE_PBR_AO;
	if(UE_ALBEDO_MAP_ENABLED) albedo = vec4(texture(UE_ALBEDO_MAP, texCoords).rgb, 1.0);
	else if(UE_DIFFUSE_MAP_ENABLED) albedo = texture(UE_DIFFUSE_MAP, texCoords).rgba;

	if(UE_APLHA_DISCARD_ENABLED && albedo.a < UE_APLHA_DISCARD_OFFSET)
		discard;

	if(UE_ROUGHNESS_MAP_ENABLED) roughness = texture(UE_ROUGHNESS_MAP, texCoords).r;
	if(UE_METALLIC_MAP_ENABLED) metallic = texture(UE_METALLIC_MAP, texCoords).r;
	if(UE_AO_MAP_ENABLED) ao = texture(UE_AO_MAP, texCoords).r;


	// store the fragment position vector in the first gbuffer texture
	gPositionShadow.rgb = fs_in.FragPos - (depth * fs_in.Normal);
	gPositionShadow.a = float(UE_ENABLE_SHADOW && UE_RECEIVE_SHADOW);

	// also store the per-fragment normals into the gbuffer
	gNormalShininess.rgb = (gl_FrontFacing ? 1.0 : -1.0) * normalize(normal);
	
	// store specular intensity in gAlbedoSpecular's alpha component
	float specular = 1.0;
	if(UE_SPECULAR_MAP_ENABLED){
		specular = texture(UE_SPECULAR_MAP, texCoords).r;
	}

	gAlbedoSpecular = vec4(albedo.rgb, specular);
	gMetallicRoughnessAO = vec4(metallic, roughness, ao, 1.0);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
	// number of depth layers
	const float minLayers = 8;
	const float maxLayers = 128;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
	// calculate the size of each layer
	float layerDepth = 1.0 / numLayers;
	// depth of current layer
	float currentLayerDepth = 0.0;
	// the amount to shift the texture coordinates per layer (from vector P)
	vec2 P = viewDir.xy / viewDir.z * UE_PBR_DISP_SCALE; 
	vec2 deltaTexCoords = P / numLayers;
	// get initial values
	vec2  currentTexCoords = texCoords;
	float currentDepthMapValue = texture(UE_DISPLACEMENT_MAP, currentTexCoords).r;
	if(UE_PBR_DISP_SCALE < 0) {
		currentDepthMapValue = 1.0 - currentDepthMapValue;
	}
	while(currentLayerDepth < currentDepthMapValue)
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		// get depthmap value at current texture coordinates
		currentDepthMapValue = texture(UE_DISPLACEMENT_MAP, currentTexCoords).r;
		
		// get depth of next layer
		currentLayerDepth += layerDepth;
	}
	// get texture coordinates before collision (reverse operations)
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
	// get depth after and before collision for linear interpolation
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(UE_DISPLACEMENT_MAP, prevTexCoords).r - currentLayerDepth + layerDepth;
	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
	return finalTexCoords;
}
