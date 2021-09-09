layout (location = 0) out vec4 FragColor;
in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D originalColor;
uniform sampler2D reflectedColorVisibility;


uniform int diffusion;
uniform int jump;
void main()
{
    vec3 color = texture(originalColor, fs_in.TexCoords).rgb;

    vec2 texelSize = 1.0 / vec2(textureSize(reflectedColorVisibility, 0));
    float visibility = 0.0;
    int count = 0;
    for (int x = -diffusion; x <= diffusion; ++x)
    {
        for (int y = -diffusion; y <= diffusion; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize * jump;
            float add = texture(reflectedColorVisibility, fs_in.TexCoords + offset).a;
            visibility += add;
            count += add == 0 ? 0 : 1;
        }
    }
    visibility /= count;
    if(texture(reflectedColorVisibility, fs_in.TexCoords).a == 0) visibility = 0;

    vec3 reflected = clamp(texture(reflectedColorVisibility, fs_in.TexCoords).rgb, vec3(0, 0, 0), vec3(1, 1, 1));

    vec3 result = color * (1.0 - visibility) + reflected * visibility;
    FragColor = vec4(result, 1.0);
}