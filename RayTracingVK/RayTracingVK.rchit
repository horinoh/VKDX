#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_scalar_block_layout : enable

layout(location = 0) rayPayloadInEXT vec3 Payload;
hitAttributeEXT vec2 HitAttr;

layout(binding = 0, set = 0) uniform accelerationStructureEXT TLAS;
struct VertexPN 
{
    vec3 Position; 
    vec3 Normal;
};
layout(binding = 2, set = 0) buffer VertexBuffer { VertexPN Vertices[]; } VB;
layout(binding = 3, set = 0) buffer IndexBuffer { uvec3 Indices[]; } IB;

layout(shaderRecordEXT) buffer SBT {
    vec4 v;
};

void main()
{
    const vec3 BaryCentric = vec3(1.0f - HitAttr.x - HitAttr.y, HitAttr.x, HitAttr.y);

    const uvec3 i = IB.Indices[gl_PrimitiveID];
    const vec3 HitPos = VB.Vertices[i.x].Position * BaryCentric.x + VB.Vertices[i.y].Position * BaryCentric.y + VB.Vertices[i.z].Position * BaryCentric.z;
    const vec3 HitNrm = VB.Vertices[i.x].Normal * BaryCentric.x + VB.Vertices[i.y].Normal * BaryCentric.y + VB.Vertices[i.z].Normal * BaryCentric.z;

//    Payload = vec3(0.0f);
//    const float TMin = 0.001f;
//    const float TMax = 100000.0f;
//    const vec3 Origin = vec4(HitPos, 1.0f) * gl_ObjectToWorld3x4EXT ;
//    const vec3 Direction = reflect(gl_WorldRayDirectionEXT, vec4(HitNrm, 0.0f) * gl_ObjectToWorld3x4EXT);
//    traceRayEXT(TLAS, gl_RayFlagsNoneEXT, 0xff, 0, 0, 0, Origin, TMin, Direction, TMax, 0);

    Payload = HitNrm * 0.5f + 0.5f;
    //Payload = v.xyz;
}