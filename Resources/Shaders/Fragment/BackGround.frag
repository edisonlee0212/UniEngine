out vec4 FragColor;

in vec3 TexCoords;

uniform vec3 clearColor;
void main()
{    
    FragColor = vec4(clearColor, 1.0);
}