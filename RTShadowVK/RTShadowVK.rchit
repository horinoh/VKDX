#version 460
#extension GL_EXT_ray_tracing : enable

struct PAYLOAD 
{
    vec3 Color;
    int Recursive;
};
layout(location = 0) rayPayloadInEXT PAYLOAD Payload;
hitAttributeEXT vec2 HitAttr;

layout(binding = 0, set = 0) uniform accelerationStructureEXT TLAS;
struct PAYLOAD_SHADOW
{
    bool IsOccluded;
};
layout(location = 1) rayPayloadInEXT PAYLOAD_SHADOW PayloadShadow;

//struct VertexP
//{
//    vec3 Position; 
//};
//layout(binding = 4, set = 0) readonly buffer VertexBuffer { VertexP Vertices[]; } VB;
//layout(binding = 5, set = 0) readonly buffer IndexBuffer { uvec3 Indices[]; } IB;

vec3 ToBaryCentric(const vec2 Attr)
{
    return vec3(1.0f - Attr.x - Attr.y, Attr.x, Attr.y);
}
vec3 ToCartesian(const vec3 A, const vec3 B, const vec3 C, const vec3 BaryCentric)
{
    return BaryCentric.x * A + BaryCentric.y * B + BaryCentric.z * C;
}
vec3 ToCartesian(const vec3 V[3], const vec3 BaryCentric)
{
    return ToCartesian(V[0], V[1], V[2], BaryCentric);
}

bool ShadowRay(const vec3 Origin, const vec3 Direction)
{
    const float TMin = 0.001f; 
    const float TMax = 100000.0f;

    PayloadShadow.IsOccluded = false;

    traceRayEXT(TLAS, 
        gl_RayFlagsOpaqueEXT /*| gl_RayFlagsCullBackFacingTrianglesEXT*/ | gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipClosestHitShaderEXT, 
        0xff,
        0,      
        0,      
        1, //!< 1 番のミスシェーダ      
        Origin, TMin, Direction, TMax, 
        1);    

    return PayloadShadow.IsOccluded;
}

const vec3 LightPos = vec3(0.0f, 100.0f, 0.0f);

void main()
{
    if(Payload.Recursive++ >= 1) {
        Payload.Color = vec3(0.0f, 1.0f, 0.0f);
        return;
    }
	Payload.Color = ToBaryCentric(HitAttr);

//    const vec3 Pos = ToCartesian(vec3(0, -1, 0), vec3(0, -1, 0), vec3(0, -1, 0), ToBaryCentric(HitAttr));
//    if(ShadowRay(Pos, normalize(LightPos - Pos))) {
//        Payload.Color *= 0.1f;
//    }
}