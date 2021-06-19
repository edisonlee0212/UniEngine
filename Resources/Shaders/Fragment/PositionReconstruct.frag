layout (location = 0) out vec3 FragColor;

in VS_OUT {
	vec2 TexCoords;
	vec2 ViewRay;
} fs_in;
uniform sampler2D inputTex;
void main(){
	//FragColor = vec3(texture(inputTex, fs_in.TexCoords).x);
	//FragColor = vec3(UE_LINEAR_DEPTH(fs_in.TexCoords, inputTex) / UE_CAMERA_RESERVED.y);
	FragColor = (inverse(UE_CAMERA_PROJECTION) * inverse(UE_CAMERA_VIEW) * vec4(UE_CALCULATE_VIEWSPACE_POS(fs_in.ViewRay, fs_in.TexCoords, inputTex), 1.0)).xyz;
}