#version 460

//!< ���� EXT -> NV �ɕς��Ă����Ȃ��ƃR���p�C�����ʂ�Ȃ��c

#extension GL_NV_ray_tracing : enable
//#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInNV vec3 Payload;
//layout(location = 0) rayPayloadInEXT vec3 Payload;

void main()
{
    Payload = vec3(0.529411793f, 0.807843208f, 0.921568692f);
}