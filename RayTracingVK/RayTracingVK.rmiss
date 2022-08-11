#version 460
#extension GL_EXT_ray_tracing : enable

struct PAYLOAD 
{
    vec3 Color;
    int Recursive;
};
layout(location = 0) rayPayloadInEXT PAYLOAD Payload;

layout (binding = 2, set = 0) uniform samplerCube CubeMap;

void main()
{
    //!< �����I�Ƀ~�b�v�}�b�v������ł��Ȃ��̂� textureLod() �Ŗ����I�Ƀ~�b�v���x�����w�肷��
    Payload.Color = textureLod(CubeMap, gl_WorldRayDirectionEXT, 0.0f).rgb;
}