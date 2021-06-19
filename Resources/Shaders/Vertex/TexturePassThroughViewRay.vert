layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out VS_OUT {
	vec2 TexCoords;
	vec2 ViewRay;
} vs_out;

void main()
{
	vs_out.TexCoords = aTexCoords;
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
	vs_out.ViewRay = vec2(aPos.x * UE_CAMERA_RESERVED.w * UE_CAMERA_RESERVED.z, aPos.y * UE_CAMERA_RESERVED.z);
}  