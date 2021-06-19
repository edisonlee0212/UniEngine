layout (location = 0) out vec4 vFragColor;

in GS_OUT {
    vec2 TexCoords;
	flat int splitIndex;
} vs_in;
//uniform
uniform sampler2DArray textureMapArray;	//the input image to blur

uniform int passIndex;
uniform int lightIndex;
void main()
{ 
	//get the inverse of texture size
	float delta = 1.0 / textureSize(textureMapArray, 0).y;
	int shift = 1;
	for(int i = 0; i < passIndex; i++){
		shift *= 2;
	}
	float color = texture(textureMapArray, vec3(vs_in.TexCoords.x, vs_in.TexCoords.y, lightIndex * 4 + vs_in.splitIndex)).r;
	if(vs_in.TexCoords.y > shift * delta) color += texture(textureMapArray, vec3(vs_in.TexCoords.x, vs_in.TexCoords.y - shift * delta, lightIndex * 4 + vs_in.splitIndex)).r;
	//return the filtered colour as fragment output
	vFragColor = vec4(color, 0, 0, 1);
}