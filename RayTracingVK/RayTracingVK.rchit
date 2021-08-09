#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 Payload;
layout(location = 0) callableDataEXT vec3 CallableData;
hitAttributeEXT vec2 HitAttr;

void main()
{
	//!< gl_InstanceCustomIndexEXT : TLAS ì¬Žž‚Ì VkAccelerationStructureInstanceKHR.instanceCustomIndex 
	Payload = vec3(1.0f - HitAttr.x - HitAttr.y, HitAttr.x, HitAttr.y);

	//executeCallableEXT(gl_GeometryIndexEXT, 0);
	const uint RecordIndex = 0; //!< ‚±‚±‚Å‚Í [0, 2]
	executeCallableEXT(RecordIndex, 0);
	Payload = CallableData;
}