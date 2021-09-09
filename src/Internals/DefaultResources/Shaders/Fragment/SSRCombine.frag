layout (location = 0) out vec4 FragColor;
in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D originalColor;
uniform sampler2D reflectedColorMetallic;
void main()
{
    float metallic = texture(reflectedColorMetallic, fs_in.TexCoords).a;
    vec3 color = texture(originalColor, fs_in.TexCoords).rgb;
    vec3 reflected = texture(reflectedColorMetallic, fs_in.TexCoords).rgb;

    vec3 result = color * (1.0 - metallic) + reflected * metallic;
    FragColor = vec4(result, 1.0);
}