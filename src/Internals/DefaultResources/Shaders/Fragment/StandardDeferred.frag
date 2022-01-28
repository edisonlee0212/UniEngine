layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec3 gAlbedo;
layout (location = 2) out vec4 gMetallicRoughnessEmissionAmbient;

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
    if (UE_ALBEDO_MAP_ENABLED) albedo = texture(UE_ALBEDO_MAP, texCoords);
    if (albedo.a < 0.05)
        discard;

    vec3 B = cross(fs_in.Normal, fs_in.Tangent);
    mat3 TBN = mat3(fs_in.Tangent, B, fs_in.Normal);
    vec3 normal = fs_in.Normal;
    if (UE_NORMAL_MAP_ENABLED){
        normal = texture(UE_NORMAL_MAP, texCoords).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(TBN * normal);
    }

    float roughness = UE_PBR_ROUGHNESS;
    float metallic = UE_PBR_METALLIC;
    float emission = UE_PBR_EMISSION;
    float ao = UE_PBR_AO;

    if (UE_ROUGHNESS_MAP_ENABLED) roughness = texture(UE_ROUGHNESS_MAP, texCoords).r;
    if (UE_METALLIC_MAP_ENABLED) metallic = texture(UE_METALLIC_MAP, texCoords).r;
    if (UE_AO_MAP_ENABLED) ao = texture(UE_AO_MAP, texCoords).r;

    // also store the per-fragment normals into the gbuffer
    gNormal.rgb = normalize((gl_FrontFacing ? 1.0 : -1.0) * normal);
    gAlbedo = vec3(albedo);
    gMetallicRoughnessEmissionAmbient = vec4(metallic, roughness, emission, ao);
}