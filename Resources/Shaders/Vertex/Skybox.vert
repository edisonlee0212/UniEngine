layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    vec4 pos = UE_CAMERA_PROJECTION * mat4(mat3(UE_CAMERA_VIEW)) * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  