#version 460
#extension GL_EXT_ray_tracing : enable

#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_scalar_block_layout : enable

//layout(binding = 0, set = 0) uniform accelerationStructureEXT TLAS;
layout(location = 0) rayPayloadInEXT vec3 Payload;
hitAttributeEXT vec2 HitAttr;

//layout(buffer_reference, buffer_reference_align=8, scalar) buffer VertexBuffer { vec3 Vertices[]; };
//layout(buffer_reference, buffer_reference_align=8, scalar) buffer IndexBuffer { uint Indices[]; };
//layout(shaderRecordEXT) buffer SBT {
//    VertexBuffer VB;
//    IndexBuffer IB;
//};

void main()
{
//    vec3 Pos[3];
//    for (int i = 0; i < 3; ++i) {
//        Pos[i] = VB.Vertices[IB.Indices[gl_PrimitiveID * 3 + i]];
//    }

	Payload = vec3(1.0f - HitAttr.x - HitAttr.y, HitAttr.x, HitAttr.y);

//	  Payload1 = vec3(0.0f);
//    const float TMin = 0.001f;
//    const float TMax = 100000.0f;
//    const vec3 Origin = mul(float4(CD.Position, 1.0f), ObjectToWorld4x3());
//    const vec3 Direction = reflect(WorldRayDirection(), mul(CD.Normal, (float3x3)ObjectToWorld4x3()));
//    traceRayEXT(TLAS, gl_RayFlagsNoneEXT, 0xff, 0, 0, 0, Origin, TMin, Direction, TMax, 0);
}