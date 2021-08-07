#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 Payload;
hitAttributeEXT vec2 HitAttr;

void main()
{
	//!< gl_InstanceCustomIndexEXT : TLAS ì¬Žž‚Ì VkAccelerationStructureInstanceKHR.instanceCustomIndex 
	Payload = vec3(1.0f - HitAttr.x - HitAttr.y, HitAttr.x, HitAttr.y);
}