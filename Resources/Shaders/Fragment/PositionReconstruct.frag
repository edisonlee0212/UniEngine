layout (location = 0) out vec3 FragColor;

in VS_OUT {
	vec2 TexCoords;
	vec2 ViewRay;
} fs_in;

uniform sampler2D inputTex;

float LinearizeDepth(float depth)
{
	float z = depth;// * 2.0 - 1.0; // back to NDC
	float near = UE_CAMERA_RESERVED[0];
	float far = UE_CAMERA_RESERVED[1];
	return (2.0 * near * far) / (far + near - z * (far - near));
}

float CalcViewZ(vec2 Coords)
{
	float Depth = texture(inputTex, Coords).x;
	float ViewZ = LinearizeDepth(Depth);
	return ViewZ;
}

void main(){
	float ViewZ = CalcViewZ(fs_in.TexCoords);

	float ViewX = fs_in.ViewRay.x * ViewZ;
	float ViewY = fs_in.ViewRay.y * ViewZ;

	vec3 Pos = vec3(ViewX, ViewY, ViewZ);
	FragColor = (inverse(UE_CAMERA_PROJECTION) * inverse(UE_CAMERA_VIEW) * vec4(Pos, 1.0)).xyz;
	//FragColor = (vec4(Pos, 0.0) * ).xyz;
	//FragColor = vec3(texture(inputTex, fs_in.TexCoords).x);
	//FragColor = vec3(ViewZ / UE_CAMERA_RESERVED[1]);
	//FragColor = Pos  / UE_CAMERA_RESERVED[1];
}