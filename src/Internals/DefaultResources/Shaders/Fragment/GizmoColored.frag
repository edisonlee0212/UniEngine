out vec4 FragColor;

in VS_OUT {
	vec4 surfaceColor;
} fs_in;

void main()
{	
	FragColor = fs_in.surfaceColor;
}