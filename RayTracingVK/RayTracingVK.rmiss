#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 Payload;

layout (binding = 4, set = 0) uniform samplerCube CubeMap;

void main()
{
    //!< �����I�Ƀ~�b�v�}�b�v������ł��Ȃ��̂� textureLod() �Ŗ����I�Ɏw�肷��
    Payload = textureLod(CubeMap, gl_WorldRayDirectionEXT, 0.0f).rgb;
}