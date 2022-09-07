#version 460
#extension GL_EXT_ray_tracing : enable

struct PAYLOAD 
{
    vec3 Color;
    int Recursive;
};
//!< rayPayloadEXT ‚Å‚Í‚È‚­‚Ä rayPayloadInEXT ‚Å‚ ‚é‚±‚Æ‚É’ˆÓ
layout(location = 0) rayPayloadInEXT PAYLOAD Payload;

struct VertexNT
{
    vec3 Normal;
    vec2 Texcoord;
};
hitAttributeEXT VertexNT HitAttr;

void main()
{
    if(Payload.Recursive++ >= 1) {
        Payload.Color = vec3(0.0f, 1.0f, 0.0f);
        return;
    }
    Payload.Color = vec3(1.0f, 1.0f, 1.0f);
}