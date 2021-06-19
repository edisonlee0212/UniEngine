out vec4 FragColor;

in VS_OUT {
    vec2 TexCoords;
} vs_in;

uniform sampler2D screenTexture;

void main()
{
    vec3 col = texture(screenTexture, vs_in.TexCoords).rgb;
    FragColor = vec4(col, 1.0);
} 