#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 Payload;
layout(location = 0) callableDataEXT vec3 CallableData;
hitAttributeEXT vec2 HitAttr;

void main()
{
	//!< [C++] VkAccelerationStructureInstanceKHR.instanceCustomIndex, [GLSL] gl_InstanceCustomIndexEXT
	executeCallableEXT(gl_InstanceCustomIndexEXT, 0);

	//!< Ô
	Payload = CallableData * vec3(1.0f, 0.0f, 0.0f);
}