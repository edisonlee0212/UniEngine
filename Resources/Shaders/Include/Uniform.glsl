
//Lights
struct DirectionalLight {
	vec3 direction;
	vec4 diffuse;
	vec3 specular;
	mat4 lightSpaceMatrix[4];
	vec4 lightFrustumWidth;
	vec4 lightFrustumDistance;
	vec4 ReservedParameters;
	int viewPortXStart;
	int viewPortYStart;
	int viewPortXSize;
	int viewPortYSize;
};

struct PointLight {
	vec3 position;
	vec4 constantLinearQuadFarPlane;
	vec4 diffuse;
	vec3 specular;
	mat4 lightSpaceMatrix[6];
	vec4 ReservedParameters;
	int viewPortXStart;
	int viewPortYStart;
	int viewPortXSize;
	int viewPortYSize;
};

struct SpotLight {
	vec3 position;
	float SpotLightPadding0;

	vec3 direction;
	float SpotLightPadding1;

	mat4 lightSpaceMatrix;
	vec4 cutOffOuterCutOffLightSizeBias;
	vec4 constantLinearQuadFarPlane;
	vec4 diffuse;
	vec3 specular;
	float SpotLightPadding3;
	int viewPortXStart;
	int viewPortYStart;
	int viewPortXSize;
	int viewPortYSize;
};

layout (std140, binding = 1) uniform UE_DIRECTIONAL_LIGHT_BLOCK
{
	int UE_DIRECTIONAL_LIGHT_BLOCK_AMOUNT;
	DirectionalLight UE_DIRECTIONAL_LIGHTS[DIRECTIONAL_LIGHTS_AMOUNT];
};

layout (std140, binding = 2) uniform UE_POINT_LIGHT_BLOCK
{
	int UE_POINT_LIGHT_AMOUNT;
	PointLight UE_POINT_LIGHTS[POINT_LIGHTS_AMOUNT];
};

layout (std140, binding = 3) uniform UE_SPOT_LIGHT_BLOCK
{
	int UE_SPOT_LIGHT_AMOUNT;
	SpotLight UE_SPOT_LIGHTS[SPOT_LIGHTS_AMOUNT];
};

layout (std140, binding = 4) uniform UE_LIGHT_SETTINGS_BLOCK
{
	float UE_SHADOW_SPLIT_0;
	float UE_SHADOW_SPLIT_1;
	float UE_SHADOW_SPLIT_2;
	float UE_SHADOW_SPLIT_3;
	int UE_SHADOW_SAMPLE_SIZE;
	float UE_SHADOW_PCSS_DIRECTIONAL_LIGHT_SCALE;
	int UE_SHADOW_PCSS_BLOCKER_SEARCH_SIZE;
	float UE_SHADOW_SEAM_FIX_RATIO;
	float UE_SHADOW_VSM_VARIANCE;
	float UE_SHADOW_LIGHT_BLEED_FACTOR;
	float UE_GAMMA;
	float UE_AMBIENT_LIGHT;
};

layout (std140, binding = 5) uniform UE_KERNEL_BLOCK
{
	vec4 UE_UNIFORM_KERNEL[MAX_KERNEL_AMOUNT];
	vec4 UE_GAUSS_KERNEL[MAX_KERNEL_AMOUNT];
};

layout (std140, binding = 8) buffer UE_ANIM_BONES_BLOCK
{
	mat4 UE_ANIM_BONES[];
};

vec3 UE_FUNC_CALCULATE_LIGHTS(bool calculateShadow, vec3 albedo, float specular, float dist, vec3 normal, vec3 viewDir, vec3 fragPos, float metallic, float roughness, vec3 F0);
vec3 UE_FUNC_DIRECTIONAL_LIGHT(vec3 albedo, float specular, int i, vec3 normal, vec3 viewDir, float metallic, float roughness, vec3 F0);
vec3 UE_FUNC_POINT_LIGHT(vec3 albedo, float specular, int i, vec3 normal, vec3 fragPos, vec3 viewDir, float metallic, float roughness, vec3 F0);
vec3 UE_FUNC_SPOT_LIGHT(vec3 albedo, float specular, int i, vec3 normal, vec3 fragPos, vec3 viewDir, float metallic, float roughness, vec3 F0);
float UE_FUNC_DIRECTIONAL_LIGHT_SHADOW(int i, int splitIndex, vec3 fragPos, vec3 normal);
float UE_FUNC_POINT_LIGHT_SHADOW(int i, vec3 fragPos, vec3 normal);
float UE_FUNC_SPOT_LIGHT_SHADOW(int i, vec3 fragPos, vec3 normal);
float UE_LINEARIZE_DEPTH(float ndcDepth);

vec3 UE_DEPTH_TO_CLIP_POS(vec2 texCoords, float ndcDepth);
vec3 UE_DEPTH_TO_WORLD_POS(vec2 texCoords, float ndcDepth);
vec3 UE_DEPTH_TO_VIEW_POS(vec2 texCoords, float ndcDepth);

float UE_LINEARIZE_DEPTH(float ndcDepth)
{
	float near = UE_CAMERA_RESERVED[0];
	float far = UE_CAMERA_RESERVED[1];
	float z = ndcDepth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

vec3 UE_DEPTH_TO_WORLD_POS(vec2 texCoords, float ndcDepth){
	vec4 viewPos = vec4(UE_DEPTH_TO_VIEW_POS(texCoords, ndcDepth), 1.0);
	vec4 worldPos = inverse(UE_CAMERA_VIEW) * viewPos;
	return worldPos.xyz;
}

vec3 UE_DEPTH_TO_VIEW_POS(vec2 texCoords, float ndcDepth){
	vec4 clipPos = vec4(UE_DEPTH_TO_CLIP_POS(texCoords, ndcDepth), 1.0);
	vec4 viewPos = inverse(UE_CAMERA_PROJECTION) * clipPos;
	viewPos = viewPos / viewPos.w;
	return viewPos.xyz;
}

vec3 UE_DEPTH_TO_CLIP_POS(vec2 texCoords, float ndcDepth){
	vec4 clipPos = vec4(texCoords * 2 - vec2(1), ndcDepth * 2.0 - 1.0, 1.0);
	return clipPos.xyz;
}

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float UE_FUNC_DISTRIBUTION_GGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;

	float nom   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / max(denom, 0.001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}
// ----------------------------------------------------------------------------
float UE_FUNC_GEOMETRY_SCHLICK_GGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float UE_FUNC_GEOMETRY_SMITH(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = UE_FUNC_GEOMETRY_SCHLICK_GGX(NdotV, roughness);
	float ggx1 = UE_FUNC_GEOMETRY_SCHLICK_GGX(NdotL, roughness);

	return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 UE_FUNC_FRESNEL_SCHLICK(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

// ----------------------------------------------------------------------------
vec3 UE_FUNC_FRESNEL_SCHLICK_ROUGHNESS(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}   

vec3 UE_FUNC_CALCULATE_ENVIRONMENTAL_LIGHT(vec3 albedo, vec3 normal, vec3 viewDir, float metallic, float roughness, vec3 F0)
{
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = UE_FUNC_FRESNEL_SCHLICK_ROUGHNESS(max(dot(normal, viewDir), 0.0), F0, roughness);
    vec3 R = reflect(-viewDir, normal); 
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = UE_ENVIRONMENTAL_BACKGROUND_COLOR.w == 1.0 ? UE_ENVIRONMENTAL_BACKGROUND_COLOR.xyz : texture(UE_ENVIRONMENTAL_IRRADIANCE, normal).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = UE_ENVIRONMENTAL_BACKGROUND_COLOR.w == 1.0 ? UE_ENVIRONMENTAL_BACKGROUND_COLOR.xyz : textureLod(UE_ENVIRONMENTAL_PREFILERED, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(UE_ENVIRONMENTAL_BRDFLUT, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
	vec3 ambient = kD * diffuse + specular;
    return ambient;
}

vec3 UE_FUNC_CALCULATE_LIGHTS(bool calculateShadow, vec3 albedo, float specular, float dist, vec3 normal, vec3 viewDir, vec3 fragPos, float metallic, float roughness, vec3 F0){
	vec3 result = vec3(0.0, 0.0, 0.0);

	// phase 1: directional lighting
	for(int i = 0; i < UE_DIRECTIONAL_LIGHT_BLOCK_AMOUNT; i++){
		float shadow = 1.0;
		if(calculateShadow && UE_DIRECTIONAL_LIGHTS[i].diffuse.w == 1.0){
			int split = 0;
			if(dist < UE_SHADOW_SPLIT_0 - UE_SHADOW_SPLIT_0 * UE_SHADOW_SEAM_FIX_RATIO){
				shadow = UE_FUNC_DIRECTIONAL_LIGHT_SHADOW(i, 0, fragPos, normal);
			}else if(dist < UE_SHADOW_SPLIT_0){
				//Blend between split 1 & 2
				shadow = UE_FUNC_DIRECTIONAL_LIGHT_SHADOW(i, 0, fragPos, normal);
				float nextLevel = UE_FUNC_DIRECTIONAL_LIGHT_SHADOW(i, 1, fragPos, normal);
				shadow = (nextLevel * (dist - (UE_SHADOW_SPLIT_0 - UE_SHADOW_SPLIT_0 * UE_SHADOW_SEAM_FIX_RATIO)) + shadow * (UE_SHADOW_SPLIT_0 - dist)) / (UE_SHADOW_SPLIT_0 * UE_SHADOW_SEAM_FIX_RATIO);
			}else if(dist < UE_SHADOW_SPLIT_1 - UE_SHADOW_SPLIT_1 * UE_SHADOW_SEAM_FIX_RATIO){
				shadow = UE_FUNC_DIRECTIONAL_LIGHT_SHADOW(i, 1, fragPos, normal);
			}else if(dist < UE_SHADOW_SPLIT_1){
				//Blend between split 2 & 3
				shadow = UE_FUNC_DIRECTIONAL_LIGHT_SHADOW(i, 1, fragPos, normal);
				float nextLevel = UE_FUNC_DIRECTIONAL_LIGHT_SHADOW(i, 2, fragPos, normal);
				shadow = (nextLevel * (dist - (UE_SHADOW_SPLIT_1 - UE_SHADOW_SPLIT_1 * UE_SHADOW_SEAM_FIX_RATIO)) + shadow * (UE_SHADOW_SPLIT_1 - dist)) / (UE_SHADOW_SPLIT_1 * UE_SHADOW_SEAM_FIX_RATIO);
			}else if(dist < UE_SHADOW_SPLIT_2 - UE_SHADOW_SPLIT_2 * UE_SHADOW_SEAM_FIX_RATIO){
				shadow = UE_FUNC_DIRECTIONAL_LIGHT_SHADOW(i, 2, fragPos, normal);
			}else if(dist < UE_SHADOW_SPLIT_2){
				//Blend between split 3 & 4
				shadow = UE_FUNC_DIRECTIONAL_LIGHT_SHADOW(i, 2, fragPos, normal);
				float nextLevel = UE_FUNC_DIRECTIONAL_LIGHT_SHADOW(i, 3, fragPos, normal);
				shadow = (nextLevel * (dist - (UE_SHADOW_SPLIT_2 - UE_SHADOW_SPLIT_2 * UE_SHADOW_SEAM_FIX_RATIO)) + shadow * (UE_SHADOW_SPLIT_2 - dist)) / (UE_SHADOW_SPLIT_2 * UE_SHADOW_SEAM_FIX_RATIO);
			}else if(dist < UE_SHADOW_SPLIT_3){
				shadow = UE_FUNC_DIRECTIONAL_LIGHT_SHADOW(i, 3, fragPos, normal);
			}else{
				shadow = 1.0;
			}
		}
		result += UE_FUNC_DIRECTIONAL_LIGHT(albedo, specular, i, normal, viewDir, metallic, roughness, F0) * shadow;
	}
	// phase 2: point lights
	for(int i = 0; i < UE_POINT_LIGHT_AMOUNT; i++){
		float shadow = 1.0;
		if(calculateShadow && UE_POINT_LIGHTS[i].diffuse.w == 1.0){
			shadow = UE_FUNC_POINT_LIGHT_SHADOW(i, fragPos, normal);
		}
		result += UE_FUNC_POINT_LIGHT(albedo, specular, i, normal, fragPos, viewDir, metallic, roughness, F0) * shadow;
	}
	// phase 3: spot light
	for(int i = 0; i < UE_SPOT_LIGHT_AMOUNT; i++){
		float shadow = 1.0;
		if(calculateShadow && UE_SPOT_LIGHTS[i].diffuse.w == 1.0){
			shadow = UE_FUNC_SPOT_LIGHT_SHADOW(i, fragPos, normal);
		}
		result += UE_FUNC_SPOT_LIGHT(albedo, specular, i, normal, fragPos, viewDir, metallic, roughness, F0) * shadow;
	}
	return result;
}

// calculates the color when using a directional light.
vec3 UE_FUNC_DIRECTIONAL_LIGHT(vec3 albedo, float specular, int i, vec3 normal, vec3 viewDir, float metallic, float roughness, vec3 F0)
{
	DirectionalLight light = UE_DIRECTIONAL_LIGHTS[i];
	vec3 lightDir = normalize(-light.direction);
	vec3 H = normalize(viewDir + lightDir);
	vec3 radiance = light.diffuse.xyz;
	float normalDF = UE_FUNC_DISTRIBUTION_GGX(normal, H, roughness);   
	float G   = UE_FUNC_GEOMETRY_SMITH(normal, viewDir, lightDir, roughness);      
	vec3 F    = UE_FUNC_FRESNEL_SCHLICK(clamp(dot(H, viewDir), 0.0, 1.0), F0);
	vec3 nominator    = normalDF * G * F; 
	float denominator = 4 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0);
	vec3 spec = nominator / max(denominator, 0.001) * specular;
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;	  
	float NdotL = max(dot(normal, lightDir), 0.0);       
	return (kD * albedo / PI + spec) * radiance * NdotL;
}

// calculates the color when using a point light.
vec3 UE_FUNC_POINT_LIGHT(vec3 albedo, float specular, int i, vec3 normal, vec3 fragPos, vec3 viewDir, float metallic, float roughness, vec3 F0)
{
	PointLight light = UE_POINT_LIGHTS[i];
	vec3 lightDir = normalize(light.position - fragPos);
	vec3 H = normalize(viewDir + lightDir);
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constantLinearQuadFarPlane.x + light.constantLinearQuadFarPlane.y * distance + light.constantLinearQuadFarPlane.z * (distance * distance));	
	vec3 radiance = light.diffuse.xyz * attenuation;
	float normalDF = UE_FUNC_DISTRIBUTION_GGX(normal, H, roughness);   
	float G   = UE_FUNC_GEOMETRY_SMITH(normal, viewDir, lightDir, roughness);      
	vec3 F    = UE_FUNC_FRESNEL_SCHLICK(clamp(dot(H, viewDir), 0.0, 1.0), F0);
	vec3 nominator    = normalDF * G * F; 
	float denominator = 4 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0);
	vec3 spec = nominator / max(denominator, 0.001) * specular;
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;	  
	float NdotL = max(dot(normal, lightDir), 0.0);       
	return (kD * albedo / PI + spec) * radiance * NdotL;

}

// calculates the color when using a spot light.
vec3 UE_FUNC_SPOT_LIGHT(vec3 albedo, float specular, int i, vec3 normal, vec3 fragPos, vec3 viewDir, float metallic, float roughness, vec3 F0)
{
	SpotLight light = UE_SPOT_LIGHTS[i];
	vec3 lightDir = normalize(light.position - fragPos);
	vec3 H = normalize(viewDir + lightDir);
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constantLinearQuadFarPlane.x + light.constantLinearQuadFarPlane.y * distance + light.constantLinearQuadFarPlane.z * (distance * distance));	
	// spotlight intensity
	float theta = dot(lightDir, normalize(-light.direction)); 
	float epsilon = light.cutOffOuterCutOffLightSizeBias.x - light.cutOffOuterCutOffLightSizeBias.y;
	float intensity = clamp((theta - light.cutOffOuterCutOffLightSizeBias.y) / epsilon, 0.0, 1.0);
	
	vec3 radiance = light.diffuse.xyz * attenuation * intensity;
	float normalDF = UE_FUNC_DISTRIBUTION_GGX(normal, H, roughness);   
	float G   = UE_FUNC_GEOMETRY_SMITH(normal, viewDir, lightDir, roughness);      
	vec3 F    = UE_FUNC_FRESNEL_SCHLICK(clamp(dot(H, viewDir), 0.0, 1.0), F0);
	vec3 nominator    = normalDF * G * F; 
	float denominator = 4 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0);
	vec3 spec = nominator / max(denominator, 0.001) * specular;
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;	  
	float NdotL = max(dot(normal, lightDir), 0.0);       
	return (kD * albedo / PI + spec) * radiance * NdotL;
}

vec2 VogelDiskSample(int sampleIndex, int sampleCount, float phi)
{
	float goldenAngle = 2.4;
	float r = sqrt(sampleIndex + 0.5) / sqrt(sampleCount);
	float theta = goldenAngle * sampleIndex + phi;
	return r * vec2(cos(theta), sin(theta));
}

float InterleavedGradientNoise(vec3 fragCoords){
	vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
	return fract(dot(fragCoords, magic));
}

float UE_FUNC_DIRECTIONAL_LIGHT_SHADOW(int i, int splitIndex, vec3 fragPos, vec3 normal)
{
	DirectionalLight light = UE_DIRECTIONAL_LIGHTS[i];
	vec3 lightDir = light.direction;
	if(dot(lightDir, normal) > -0.02) return 1.0;
	vec4 fragPosLightSpace = light.lightSpaceMatrix[splitIndex] * vec4(fragPos, 1.0);
	float bias = light.ReservedParameters.z * light.lightFrustumWidth[splitIndex] / light.viewPortXSize;
	float normalOffset = light.ReservedParameters.w * light.lightFrustumWidth[splitIndex] / light.viewPortXSize;
	// perform perspective divide
	vec3 projCoords = (fragPosLightSpace.xyz + normal * normalOffset) / fragPosLightSpace.w;
	//
	if(projCoords.z > 1.0){
		return 0.0;
	}
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// get depth of current fragment from light's perspective
	projCoords = vec3(projCoords.xy, projCoords.z - bias);
	float shadow = 0.0;
	float lightSize = light.ReservedParameters.x;
	

	int blockers = 0;
	float avgDistance = 0;

	int sampleAmount = UE_SHADOW_PCSS_BLOCKER_SEARCH_SIZE;
	float sampleWidth = lightSize / light.lightFrustumWidth[splitIndex] / sampleAmount;

	float texScale = float(light.viewPortXSize) / float(textureSize(UE_DIRECTIONAL_LIGHT_SM, 0).x);
	vec2 texBase = vec2(float(light.viewPortXStart) / float(textureSize(UE_DIRECTIONAL_LIGHT_SM, 0).y), float(light.viewPortYStart) / float(textureSize(UE_DIRECTIONAL_LIGHT_SM, 0).y));

	
	for(int i = -sampleAmount; i <= sampleAmount; i++)
	{
		for(int j = -sampleAmount; j <= sampleAmount; j++){
			vec2 texCoord = projCoords.xy + vec2(i, j) * sampleWidth;
			float closestDepth = texture(UE_DIRECTIONAL_LIGHT_SM, vec3(texCoord * texScale + texBase, splitIndex)).r;
			int tf = int(closestDepth != 0.0 && projCoords.z > closestDepth);
			avgDistance += closestDepth * tf;
			blockers += tf;
		}
	}
	if(blockers == 0) return 1.0;
	float blockerDistance = avgDistance / blockers;
	float penumbraWidth = (projCoords.z - blockerDistance) / blockerDistance * lightSize;
	float texelSize = penumbraWidth * UE_SHADOW_PCSS_DIRECTIONAL_LIGHT_SCALE / UE_DIRECTIONAL_LIGHTS[i].lightFrustumWidth[splitIndex] * UE_DIRECTIONAL_LIGHTS[i].lightFrustumDistance[splitIndex] / 100.0;
	
	int shadowCount = 0;
	sampleAmount = UE_SHADOW_SAMPLE_SIZE;
	for(int i = 0; i < sampleAmount; i++)
	{
		vec2 texCoord = projCoords.xy + VogelDiskSample(i, sampleAmount, InterleavedGradientNoise(fragPos * 3141)) * (texelSize + 0.001);
		float closestDepth = texture(UE_DIRECTIONAL_LIGHT_SM, vec3(texCoord * texScale + texBase, splitIndex)).r;
		if(closestDepth == 0.0) continue;
		shadow += projCoords.z < closestDepth ? 1.0 : 0.0;
	}
	shadow /= sampleAmount;
	return shadow;
}

float UE_FUNC_SPOT_LIGHT_SHADOW(int i, vec3 fragPos, vec3 normal){
	SpotLight light = UE_SPOT_LIGHTS[i];
	float bias = light.cutOffOuterCutOffLightSizeBias.w;
	vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);
	fragPosLightSpace.z -= bias;
	vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	float texScale = float(light.viewPortXSize) / float(textureSize(UE_SPOT_LIGHT_SM, 0).x);
	vec2 texBase = vec2(float(light.viewPortXStart) / float(textureSize(UE_SPOT_LIGHT_SM, 0).y), float(light.viewPortYStart) / float(textureSize(UE_SPOT_LIGHT_SM, 0).y));

	//Blocker Search
	int sampleAmount = UE_SHADOW_PCSS_BLOCKER_SEARCH_SIZE;
	float lightSize = light.cutOffOuterCutOffLightSizeBias.z * projCoords.z / light.cutOffOuterCutOffLightSizeBias.y;
	float blockers = 0;
	float avgDistance = 0;
	float sampleWidth = lightSize / sampleAmount;
	for(int i = -sampleAmount; i <= sampleAmount; i++)
	{
		for(int j = -sampleAmount; j <= sampleAmount; j++){
			vec2 texCoord = projCoords.xy + vec2(i, j) * sampleWidth;
			float closestDepth = texture(UE_SPOT_LIGHT_SM, vec2(texCoord * texScale + texBase)).r;
			int tf = int(closestDepth != 0.0 && projCoords.z > closestDepth);
			avgDistance += closestDepth * tf;
			blockers += tf;
		}
	}
	if(blockers == 0) return 1.0;
	float blockerDistance = avgDistance / blockers;
	float penumbraWidth = (projCoords.z - blockerDistance) / blockerDistance * lightSize * UE_SHADOW_PCSS_DIRECTIONAL_LIGHT_SCALE;
	//End search
	sampleAmount = UE_SHADOW_SAMPLE_SIZE;
	float shadow = 0.0;
	for(int i = 0; i < sampleAmount; i++)
	{
		vec2 texCoord = projCoords.xy + VogelDiskSample(i, sampleAmount, InterleavedGradientNoise(fragPos * 3141)) * (penumbraWidth + 0.001);
		float closestDepth = texture(UE_SPOT_LIGHT_SM, vec2(texCoord * texScale + texBase)).r;
		if(closestDepth == 0.0) continue;
		shadow += projCoords.z < closestDepth ? 1.0 : 0.0;
	}
	shadow /= sampleAmount;
	return shadow;
}

float UE_FUNC_POINT_LIGHT_SHADOW(int i, vec3 fragPos, vec3 normal)
{
	PointLight light = UE_POINT_LIGHTS[i];
	vec3 lightPos = light.position;
	// get vector between fragment position and light position
	vec3 fragToLight = fragPos - lightPos;
	float shadow = 0.0;
	float bias = light.ReservedParameters.x;
	vec3 direction = normalize(fragToLight);
	int slice = 0;
	if (abs(direction.x) >= abs(direction.y) && abs(direction.x) >= abs(direction.z))
	{
		if(direction.x > 0){
			slice = 0;
		}else{
			slice = 1;
		}
	}else if(abs(direction.y) >= abs(direction.z)){
		if(direction.y > 0){
			slice = 2;
		}else{
			slice = 3;
		}
	}else{
		if(direction.z > 0){
			slice = 4;
		}else{
			slice = 5;
		}
	}
	vec4 fragPosLightSpace = light.lightSpaceMatrix[slice] * vec4(fragPos, 1.0);
	fragPosLightSpace.z -= bias;
	vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	float texScale = float(light.viewPortXSize) / float(textureSize(UE_POINT_LIGHT_SM, 0).x);
	vec2 texBase = vec2(float(light.viewPortXStart) / float(textureSize(UE_POINT_LIGHT_SM, 0).y), float(light.viewPortYStart) / float(textureSize(UE_POINT_LIGHT_SM, 0).y));

	//Blocker Search
	int sampleAmount = UE_SHADOW_PCSS_BLOCKER_SEARCH_SIZE;
	float lightSize = light.ReservedParameters.y * projCoords.z;
	float blockers = 0;
	float avgDistance = 0;
	float sampleWidth = lightSize / sampleAmount;
	for(int i = -sampleAmount; i <= sampleAmount; i++)
	{
		for(int j = -sampleAmount; j <= sampleAmount; j++){
			vec2 texCoord = projCoords.xy + vec2(i, j) * sampleWidth;
			texCoord.x = clamp(texCoord.x, 1.0 / float(light.viewPortXSize), 1.0 - 1.0 / float(light.viewPortXSize));
			texCoord.y = clamp(texCoord.y, 1.0 / float(light.viewPortXSize), 1.0 - 1.0 / float(light.viewPortXSize));
			float closestDepth = texture(UE_POINT_LIGHT_SM, vec3(texCoord * texScale + texBase, slice)).r;
			int tf = int(closestDepth != 0.0 && projCoords.z > closestDepth);
			avgDistance += closestDepth * tf;
			blockers += tf;
		}
	}

	if(blockers == 0) return 1.0;
	float blockerDistance = avgDistance / blockers;
	float penumbraWidth = (projCoords.z - blockerDistance) / blockerDistance * lightSize * UE_SHADOW_PCSS_DIRECTIONAL_LIGHT_SCALE;	
	//End search
	sampleAmount = UE_SHADOW_SAMPLE_SIZE;
	for(int i = 0; i < sampleAmount; i++)
	{
		vec2 texCoord = projCoords.xy + VogelDiskSample(i, sampleAmount, InterleavedGradientNoise(fragPos * 3141)) * (penumbraWidth + 0.001);
		texCoord.x = clamp(texCoord.x, 1.0 / float(light.viewPortXSize), 1.0 - 1.0 / float(light.viewPortXSize));
		texCoord.y = clamp(texCoord.y, 1.0 / float(light.viewPortXSize), 1.0 - 1.0 / float(light.viewPortXSize));
		float closestDepth = texture(UE_POINT_LIGHT_SM, vec3(texCoord * texScale + texBase, slice)).r;
		if(closestDepth == 0.0) continue;
		shadow += projCoords.z < closestDepth ? 1.0 : 0.0;
	}
	shadow /= sampleAmount;
	return shadow;
}



