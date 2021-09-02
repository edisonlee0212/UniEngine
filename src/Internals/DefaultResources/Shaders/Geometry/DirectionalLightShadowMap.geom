layout (triangles) in;
layout (triangle_strip, max_vertices = 12) out;

out vec4 FragPos; // FragPos from GS (output per emitvertex)
uniform int index;
void main()
{
    for(int split = 0; split < 4; ++split)
    {
        gl_Layer = split; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = UE_DIRECTIONAL_LIGHTS[index].lightSpaceMatrix[split] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
} 