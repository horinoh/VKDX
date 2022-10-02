#version 460
#extension GL_EXT_ray_tracing : enable

struct PAYLOAD 
{
    vec3 Color;
    int Recursive;
};
//!< rayPayloadEXT ではなくて rayPayloadInEXT であることに注意
layout(location = 0) rayPayloadInEXT PAYLOAD Payload;
hitAttributeEXT vec2 HitAttr;

//!< v0, v1, v2 の３頂点の場合、hitAttributeEXT.xy はそれぞれ v1, v2 のウエイト
vec3 ToBaryCentric(const vec2 Attr)
{
    return vec3(1.0f - Attr.x - Attr.y, Attr.x, Attr.y);
}
//!< v = v0 + hitAttributeEXT.x * (v1 - v0) + hitAttributeEXT.y * (v2 - v0)
vec3 ToCartesian(const vec3 A, const vec3 B, const vec3 C, const vec3 BaryCentric)
{
    return BaryCentric.x * A + BaryCentric.y * B + BaryCentric.z * C;
}
vec3 ToCartesian(const vec3 V[3], const vec3 BaryCentric)
{
    return ToCartesian(V[0], V[1], V[2], BaryCentric);
}

void main()
{
    if(Payload.Recursive++ >= 1) {
        Payload.Color = vec3(0.0f, 1.0f, 0.0f);
        return;
    }
	Payload.Color = ToBaryCentric(HitAttr);
}