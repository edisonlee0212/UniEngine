layout (location = 0) out vec4 FragColor;
in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D originalColor;
uniform sampler2D reflectedColorVisibility;

void main()
{
    vec3 color = texture(originalColor, fs_in.TexCoords).rgb;
    vec4 reflected = texture(reflectedColorVisibility, fs_in.TexCoords);
    vec3 result = color * (1.0 - reflected.a) + reflected.rgb * reflected.a;
    FragColor = vec4(result, 1.0);
}