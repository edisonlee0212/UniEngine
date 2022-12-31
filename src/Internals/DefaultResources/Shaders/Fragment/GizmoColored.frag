out vec4 FragColor;

in VS_OUT {
	vec4 Color;
} fs_in;

void main()
{	
	FragColor = fs_in.Color;
}