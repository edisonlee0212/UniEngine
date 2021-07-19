out vec4 FragColor;

in VS_OUT {
	vec2 TexCoords;
	vec2 ViewRay;
} fs_in;

uniform sampler2D gPositionShadow;
uniform sampler2D gNormalDepth;
uniform sampler2D gAlbedoEmission;
uniform sampler2D gMetallicRoughnessAO;

float LinearizeDepth(float depth)
{
	float near = UE_CAMERA_RESERVED[0];
	float far = UE_CAMERA_RESERVED[1];
	float z = depth * 2.0 - 1.0; // back to NDC
	return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
	vec3 normal = texture(gNormalDepth, fs_in.TexCoords).rgb;
	float depth = texture(gNormalDepth, fs_in.TexCoords).a;

	float metallic = texture(gMetallicRoughnessAO, fs_in.TexCoords).r;
	float roughness = texture(gMetallicRoughnessAO, fs_in.TexCoords).g;
	float ao = texture(gMetallicRoughnessAO, fs_in.TexCoords).b;

	vec3 albedo = texture(gAlbedoEmission, fs_in.TexCoords).rgb;
	float emission = texture(gAlbedoEmission, fs_in.TexCoords).a;

	vec3 fragPos2 = texture(gPositionShadow, fs_in.TexCoords).rgb;
	vec3 viewDir = normalize(UE_CAMERA_POSITION - fragPos2);
	vec3 fragPos = -depth * viewDir + UE_CAMERA_POSITION;
	bool receiveShadow = bool(texture(gPositionShadow, fs_in.TexCoords).a);

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);

	vec3 result = UE_FUNC_CALCULATE_LIGHTS(receiveShadow, albedo, 1.0, depth, normal, viewDir, fragPos, metallic, roughness, F0);
	vec3 ambient = UE_FUNC_CALCULATE_ENVIRONMENTAL_LIGHT(albedo, normal, viewDir, metallic, roughness, F0);
	
	vec3 color = result + emission * normalize(albedo.xyz) + ambient * ao * UE_AMBIENT_LIGHT;

	color = pow(color, vec3(1.0 / UE_GAMMA));

	//float fragDepth

	int width = textureSize(gAlbedoEmission, 0).x;

	//FragColor = vec4(fs_in.ViewRay.x, fs_in.ViewRay.y, 0.0, 1.0);
	vec4 projectedPos = vec4(fs_in.TexCoords * 2 - vec2(1), depth * 2.0 - 1.0, 1.0); //(X, Y, Z range -1 to 1) with depth =gl_FragCoord.z;
	vec4 unprojectedPos = inverse(UE_CAMERA_PROJECTION) * projectedPos;
	unprojectedPos = unprojectedPos / unprojectedPos.w;
	unprojectedPos = inverse(UE_CAMERA_VIEW) * unprojectedPos;
	//vec4 viewSpacePos = ;

	FragColor = gl_FragCoord.x < width / 2 ? unprojectedPos : vec4(fragPos2, 1.0);
	//FragColor = vec4(fragCoord.xyz / fragCoord.w, 1.0);
	//FragColor = gl_FragCoord.x < width / 2 ? vec4(fragCoord.xyz / fragCoord.w, 1.0) : vec4(viewDir, 1.0);
	//FragColor = vec4(color, 1.0);
}