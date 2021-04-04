#version 460

//!< 現状 EXT -> NV に変えておかないとコンパイルが通らない…

#extension GL_NV_ray_tracing : enable
//#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInNV vec3 Payload;
//layout(location = 0) rayPayloadInEXT vec3 Payload;
hitAttributeNV vec3 HA;
//hitAttributeEXT vec3 HA;

void main()
{
    //!< v0, v1, v2 の３頂点の場合、hitAttributeEXT.xy はそれぞれ v1, v2 のウエイト
    //!< v = v0 + hitAttributeEXT.x * (v1 - v0) + hitAttributeEXT.y * (v2 - v0)
    const vec3 BaryCentrics = vec3(1.0f - HA.x - HA.y, HA.x, HA.y);

	Payload = BaryCentrics;
}