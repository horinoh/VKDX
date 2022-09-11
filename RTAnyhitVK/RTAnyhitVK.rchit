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
    const vec2 UV = (vec2(gl_LaunchIDEXT.xy) + 0.5f) / gl_LaunchSizeEXT.xy;
    Payload.Color = vec3(UV, 0.0f);
}