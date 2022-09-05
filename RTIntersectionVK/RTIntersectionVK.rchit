#version 460
#extension GL_EXT_ray_tracing : enable

struct PAYLOAD 
{
    vec3 Color;
    int Recursive;
};
//!< rayPayloadEXT �ł͂Ȃ��� rayPayloadInEXT �ł��邱�Ƃɒ���
layout(location = 0) rayPayloadInEXT PAYLOAD Payload;
hitAttributeEXT vec2 HitAttr;

void main()
{
    if(Payload.Recursive++ >= 1) {
        Payload.Color = vec3(0.0f, 1.0f, 0.0f);
        return;
    }

    //!< v0, v1, v2 �̂R���_�̏ꍇ�AhitAttributeEXT.xy �͂��ꂼ�� v1, v2 �̃E�G�C�g
    //!< v = v0 + hitAttributeEXT.x * (v1 - v0) + hitAttributeEXT.y * (v2 - v0)
	Payload.Color = vec3(1.0f - HitAttr.x - HitAttr.y, HitAttr.x, HitAttr.y); //!< BaryCentric
}