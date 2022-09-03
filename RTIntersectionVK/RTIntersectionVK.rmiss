#version 460
#extension GL_EXT_ray_tracing : enable

struct PAYLOAD 
{
    vec3 Color;
    int Recursive;
};
//!< rayPayloadEXT ‚Å‚Í‚È‚­‚Ä rayPayloadInEXT ‚Å‚ ‚é‚±‚Æ‚É’ˆÓ
layout(location = 0) rayPayloadInEXT PAYLOAD Payload;

void main()
{
    Payload.Color = vec3(0.529411793f, 0.807843208f, 0.921568692f);
}