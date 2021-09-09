layout (location = 0) out vec3 originalColor;
layout (location = 1) out vec4 colorVisibility;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D gBufferMetallicRoughnessEmissionAmbient;
uniform sampler2D colorTexture;
uniform sampler2D gBufferDepth;
uniform sampler2D gBufferNormal;

vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth);

vec4 RayMarch(in vec3 dir, inout vec3 hitCoord, out float dDepth);

vec3 hash(vec3 a);

void main()
{
    float maxDistance = 8;
    float resolution  = 0.3;
    int   steps       = 5;
    float thickness   = 0.5;

    vec2 texSize  = textureSize(colorTexture, 0).xy;
    vec2 texCoord = fs_in.TexCoords;

    vec2 uv = vec2(0.0);


    float metallic = texture(gBufferMetallicRoughnessEmissionAmbient, texCoord).r;
    vec3 albedo = texture(colorTexture, texCoord).rgb;

    vec3 positionFrom = UE_DEPTH_TO_WORLD_POS(texCoord, texture(gBufferDepth, texCoord).x);


    vec3 unitPositionFrom = normalize(positionFrom.xyz);
    vec3 normal           = normalize(texture(gBufferNormal, texCoord).xyz);
    vec3 pivot            = normalize(reflect(unitPositionFrom, normal));

    float roughness = texture(gBufferMetallicRoughnessEmissionAmbient, texCoord).g;

    vec3 positionTo = positionFrom;

    vec3 startView = positionFrom + (pivot *         0.0);
    vec3 endView   = positionFrom + (pivot * maxDistance);

    vec4 startFrag     = UE_CAMERA_PROJECTION * vec4(startView, 1.0);
    startFrag.xyz /= startFrag.w;
    startFrag.xy   = startFrag.xy * 0.5 + 0.5;
    startFrag.xy  *= texSize;

    vec4 endFrag      = UE_CAMERA_PROJECTION * vec4(endView, 1.0);
    endFrag.xyz /= endFrag.w;
    endFrag.xy   = endFrag.xy * 0.5 + 0.5;
    endFrag.xy  *= texSize;

    vec2 frag  = startFrag.xy;
    uv.xy = frag / texSize;

    float deltaX    = endFrag.x - startFrag.x;
    float deltaY    = endFrag.y - startFrag.y;
    float useX      = abs(deltaX) >= abs(deltaY) ? 1.0 : 0.0;
    float delta     = mix(abs(deltaY), abs(deltaX), useX) * clamp(resolution, 0.0, 1.0);
    vec2  increment = vec2(deltaX, deltaY) / max(delta, 0.001);

    float search0 = 0;
    float search1 = 0;

    int hit0 = 0;
    int hit1 = 0;

    float viewDistance = startView.y;
    float depth        = thickness;

    float i = 0;

    for (i = 0; i < int(delta); ++i) {
        frag      += increment;
        uv.xy      = frag / texSize;
        positionTo = UE_DEPTH_TO_WORLD_POS(uv, texture(gBufferDepth, uv).x);//texture(positionTexture, uv.xy);

        search1 =
        mix
        ((frag.y - startFrag.y) / deltaY
        , (frag.x - startFrag.x) / deltaX
        , useX
        );

        search1 = clamp(search1, 0.0, 1.0);

        viewDistance = (startView.y * endView.y) / mix(endView.y, startView.y, search1);
        depth        = viewDistance - positionTo.y;

        if (depth > 0 && depth < thickness) {
            hit0 = 1;
            break;
        } else {
            search0 = search1;
        }
    }

    search1 = search0 + ((search1 - search0) / 2.0);

    steps *= hit0;

    for (i = 0; i < steps; ++i) {
        frag       = mix(startFrag.xy, endFrag.xy, search1);
        uv.xy      = frag / texSize;
        positionTo = UE_DEPTH_TO_WORLD_POS(uv, texture(gBufferDepth, uv).x);

        viewDistance = (startView.y * endView.y) / mix(endView.y, startView.y, search1);
        depth        = viewDistance - positionTo.y;

        if (depth > 0 && depth < thickness) {
            hit1 = 1;
            search1 = search0 + ((search1 - search0) / 2);
        } else {
            float temp = search1;
            search1 = search1 + ((search1 - search0) / 2);
            search0 = temp;
        }
    }

    float visibility =
    hit1
    //* positionTo.w
    * ( 1
    - max
    ( dot(-unitPositionFrom, pivot)
    , 0
    )
    )
    * ( 1
    - clamp
    ( depth / thickness
    , 0
    , 1
    )
    )
    * ( 1
    - clamp
    (   length(positionTo - positionFrom)
    / maxDistance
    , 0
    , 1
    )
    )
    * (uv.x < 0 || uv.x > 1 ? 0 : 1)
    * (uv.y < 0 || uv.y > 1 ? 0 : 1);

    visibility = clamp(visibility, 0, 1);

    colorVisibility = vec4(texture(colorTexture, uv).rgb, visibility);
    originalColor = albedo;
}