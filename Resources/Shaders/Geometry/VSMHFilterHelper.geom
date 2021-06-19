layout (triangles) in;
layout (triangle_strip, max_vertices = 12) out;

in VS_OUT {
    vec2 TexCoords;
} gs_in[];

//out vec4 FragPos; // FragPos from GS (output per emitvertex)
uniform int lightIndex;
flat out int splitIndex;

out GS_OUT {
    vec2 TexCoords;
	flat int splitIndex;
} gs_out;

void main()
{
    
    for(int split = 0; split < 4; ++split)
    {
        gl_Layer = split; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            gl_Position = gl_in[i].gl_Position;
            gs_out.TexCoords = gs_in[i].TexCoords;
            gs_out.splitIndex = split;
            EmitVertex();
        }
        EndPrimitive();
    }
} 