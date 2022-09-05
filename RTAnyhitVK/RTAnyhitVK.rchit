#version 460
#extension GL_EXT_ray_tracing : enable

struct PAYLOAD 
{
    vec3 Color;
    int Recursive;
};
//!< rayPayloadEXT ‚Å‚Í‚È‚­‚Ä rayPayloadInEXT ‚Å‚ ‚é‚±‚Æ‚É’ˆÓ
layout(location = 0) rayPayloadInEXT PAYLOAD Payload;
hitAttributeEXT vec2 HitAttr;

vec3 ToCartesian(const vec3 Barycentric, const vec3 p0, const vec3 p1, const vec3 p2)
{
    return Barycentric.x * p0 + Barycentric.y * p1 + Barycentric.z * p2;
}

void main()
{
    if(Payload.Recursive++ >= 1) {
        Payload.Color = vec3(0.0f, 1.0f, 0.0f);
        return;
    }

#if 1
    Payload.Color = vec3(1.0f, 0.0f, 0.0f);
#else
    const vec3 Barycentric = vec3(1.0f - HitAttr.x - HitAttr.y, HitAttr.x, HitAttr.y);
   
    const vec3 p0 = vec3(-1.0f, 1.0f, 0.0f);
    const vec3 p1 = vec3(-1.0f, -1.0f, 0.0f);
    const vec3 p2 = vec3(1.0f, 1.0f, 0.0f);

    const vec3 Cartesian = ToCartesian(Barycentric, p0, p1, p2);

    Payload.Color = vec3(Cartesian.x, -Cartesian.y, 0.0f) * 0.5f + 0.5f;
    Payload.Color = vec3(Cartesian.x, -Cartesian.y, 0.0f);
#endif
}