layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTexCoord;

out VS_OUT {
    vec2 TexCoord;
} vs_out;

void main()
{
    vs_out.TexCoord = inTexCoord;
    gl_Position = vec4(inPosition.x, inPosition.y, 0.0, 1.0); 
}  