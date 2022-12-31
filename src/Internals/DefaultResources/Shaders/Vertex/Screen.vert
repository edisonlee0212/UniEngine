layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out VS_OUT {
    vec2 TexCoord;
} vs_out;

uniform vec2 center;
uniform vec2 size;
uniform float depth;
void main()
{
    vs_out.TexCoord = aTexCoord;
    gl_Position = vec4(center + aPos * size, 0.0, 1.0); 
}  