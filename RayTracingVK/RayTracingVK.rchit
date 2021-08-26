#version 460
#extension GL_EXT_ray_tracing : enable

//layout(binding = 0, set = 0) uniform accelerationStructureEXT TLAS;
layout(location = 0) rayPayloadInEXT vec3 Payload;
hitAttributeEXT vec2 HitAttr;

//!< LocalRootSignature‘Š“–
//layout(shaderRecordEXT) buffer XXX {
//    uint32_t yyy;
//};

void main()
{
	Payload = vec3(1.0f - HitAttr.x - HitAttr.y, HitAttr.x, HitAttr.y);

//	  Payload1 = vec3(0.0f);
//    const float TMin = 0.001f;
//    const float TMax = 100000.0f;
//    const vec3 Origin = mul(float4(CD.Position, 1.0f), ObjectToWorld4x3());
//    const vec3 Direction = reflect(WorldRayDirection(), mul(CD.Normal, (float3x3)ObjectToWorld4x3()));
//    traceRayEXT(TLAS, gl_RayFlagsNoneEXT, 0xff, 0, 0, 0, Origin, TMin, Direction, TMax, 0);
}