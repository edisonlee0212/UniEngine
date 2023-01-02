layout (location = 0) out vec3 originalColor;
layout (location = 1) out vec4 colorVisibility;

in VS_OUT {
    vec2 TexCoord;
} fs_in;

uniform sampler2D gBufferMetallicRoughnessEmissionAmbient;
uniform sampler2D colorTexture;
uniform sampler2D gBufferDepth;
uniform sampler2D gBufferNormal;


uniform float step;
uniform float minRayStep;
uniform int maxSteps;


uniform int numBinarySearchSteps;
uniform float reflectionSpecularFalloffExponent;


#define Scale vec3(.8, .8, .8)
#define K 19.19


vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth);
vec4 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth);
vec3 hash(vec3 a);

void main()
{
    vec2 texSize  = textureSize(colorTexture, 0).xy;
    vec2 texCoord = fs_in.TexCoord;

    vec2 uv = vec2(0.0);

    float metallic = texture(gBufferMetallicRoughnessEmissionAmbient, texCoord).r;
    float roughness = texture(gBufferMetallicRoughnessEmissionAmbient, texCoord).g;
    vec3 albedo = texture(colorTexture, texCoord).rgb;
    vec3 normal = texture(gBufferNormal, texCoord).rgb;
    vec3 viewNormal = (UE_CAMERA_VIEW * vec4(normal, 0.0)).xyz;

    vec3 viewPos = UE_DEPTH_TO_VIEW_POS(texCoord, texture(gBufferDepth, texCoord).x);
    vec3 worldPos = UE_DEPTH_TO_WORLD_POS(texCoord, texture(gBufferDepth, texCoord).x);
    vec3 cameraPosition = UE_CAMERA_POSITION();
    vec3 viewDir = normalize(cameraPosition - worldPos);
    vec3 F0 = vec3(0.04);
    F0      = mix(F0, albedo, metallic);
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = UE_FUNC_FRESNEL_SCHLICK_ROUGHNESS(max(dot(normal, viewDir), 0.0), F0, roughness);
    // Reflection vector
    vec3 reflected = normalize(reflect(normalize(viewPos), normalize(viewNormal)));
    vec3 hitPos = viewPos;
    float dDepth;
    vec3 jittering = mix(vec3(0.0), vec3(hash(worldPos)), roughness);
    vec4 coords = RayMarch((vec3(jittering) + reflected * max(minRayStep, -viewPos.z)), hitPos, dDepth);
    vec2 dCoord = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));
    float screenEdgefactor = clamp(1.0 - (dCoord.x + dCoord.y), 0.0, 1.0);
    float reflectionMultiplier = pow(metallic, reflectionSpecularFalloffExponent) * screenEdgefactor * -reflected.z;
    // Get color
    colorVisibility = clamp(vec4(texture(colorTexture, coords.xy).rgb, reflectionMultiplier), vec4(0, 0, 0, 0), vec4(1, 1, 1, 1));
    originalColor = albedo;
}

vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth)
{
    float depth;
    vec4 projectedCoord;
    for(int i = 0; i < numBinarySearchSteps; i++)
    {
        projectedCoord = UE_CAMERA_PROJECTION * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

        depth = UE_DEPTH_TO_VIEW_POS(projectedCoord.xy, texture(gBufferDepth, projectedCoord.xy).x).z;//textureLod(gPosition, projectedCoord.xy, 2).z;


        dDepth = hitCoord.z - depth;

        dir *= 0.5;
        if(dDepth > 0.0)
        hitCoord += dir;
        else
        hitCoord -= dir;
    }

    projectedCoord = UE_CAMERA_PROJECTION * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

    return vec3(projectedCoord.xy, depth);
}

vec4 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth)
{
    dir *= step;
    float depth;
    int steps;
    vec4 projectedCoord;
    for(int i = 0; i < maxSteps; i++)
    {
        hitCoord += dir;
        projectedCoord = UE_CAMERA_PROJECTION * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
        depth = depth = UE_DEPTH_TO_VIEW_POS(projectedCoord.xy, texture(gBufferDepth, projectedCoord.xy).x).z;
        if(depth > 1000.0)
        continue;
        dDepth = hitCoord.z - depth;
        if((dir.z - dDepth) < 1.2)
        {
            if(dDepth <= 0.0)
            {
                vec4 Result;
                Result = vec4(BinarySearch(dir, hitCoord, dDepth), 1.0);
                return Result;
            }
        }
        steps++;
    }
    return vec4(projectedCoord.xy, depth, 0.0);
}

vec3 hash(vec3 a)
{
    a = fract(a * Scale);
    a += dot(a, a.yxz + K);
    return fract((a.xxy + a.yxx)*a.zyx);
}