out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Tangent;
    vec2 TexCoord;
    vec4 Color;
} fs_in;


void main()
{
    vec2 texCoord = fs_in.TexCoord;
    vec4 albedo = UE_PBR_ALBEDO;
    float albedoAlpha = albedo.a;
    if (UE_ALBEDO_MAP_ENABLED) {
        albedo = texture(UE_ALBEDO_MAP, texCoord);
        albedo.a = albedo.a * albedoAlpha;
    }
    if (albedo.a < 0.05)
        discard;

    albedo.xyz = vec3(albedo.xyz * (1.0 - fs_in.Color.w) + fs_in.Color.xyz * fs_in.Color.w);
    
    vec3 B = cross(fs_in.Normal, fs_in.Tangent);
    mat3 TBN = mat3(fs_in.Tangent, B, fs_in.Normal);
    vec3 normal = fs_in.Normal;
    if (UE_NORMAL_MAP_ENABLED){
        normal = texture(UE_NORMAL_MAP, texCoord).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(TBN * normal);
    }

    float roughness = UE_PBR_ROUGHNESS;
    float metallic = UE_PBR_METALLIC;
    float emission = UE_PBR_EMISSION;
    float ao = UE_PBR_AO;

    if (UE_ROUGHNESS_MAP_ENABLED) roughness = texture(UE_ROUGHNESS_MAP, texCoord).r;
    if (UE_METALLIC_MAP_ENABLED) metallic = texture(UE_METALLIC_MAP, texCoord).r;
    if (UE_AO_MAP_ENABLED) ao = texture(UE_AO_MAP, texCoord).r;
    vec3 cameraPosition = UE_CAMERA_POSITION();
    vec3 viewDir = normalize(cameraPosition - fs_in.FragPos);
    float dist = distance(fs_in.FragPos, cameraPosition);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.rgb, metallic);
    vec3 result = UE_FUNC_CALCULATE_LIGHTS(UE_ENABLE_SHADOW && UE_RECEIVE_SHADOW, albedo.rgb, 1.0, dist, normal, viewDir, fs_in.FragPos, metallic, roughness, F0);
    vec3 ambient = UE_FUNC_CALCULATE_ENVIRONMENTAL_LIGHT(albedo.rgb, normal, viewDir, metallic, roughness, F0);
    vec3 color = result + emission * normalize(albedo.xyz) + ambient * ao;

    color = pow(color, vec3(1.0 / UE_GAMMA));

    FragColor = vec4(color, albedo.a);
}