layout (location = 0) out vec4 FragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D image;
uniform int diffusion;
uniform bool horizontal;

void main()
{
    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec4 result = vec4(0.0, 0.0, 0.0, 0.0);
    if(horizontal)
    {
        for(int i = -diffusion; i <= diffusion; ++i)
        {
            result += texture(image, fs_in.TexCoords + vec2(tex_offset.x * i, 0.0));
        }
    }
    else
    {
        for(int i = -diffusion; i <= diffusion; ++i)
        {
            result += texture(image, fs_in.TexCoords + vec2(0.0, tex_offset.y * i));
        }
    }
    FragColor = result / (2 * diffusion + 1);
}