#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 Payload;
layout(location = 0) callableDataEXT vec3 CallableData;
hitAttributeEXT vec2 HitAttr;

void main()
{
	Payload = vec3(1.0f - HitAttr.x - HitAttr.y, HitAttr.x, HitAttr.y);

	executeCallableEXT(gl_InstanceCustomIndexEXT, 0);

	Payload = CallableData * vec3(0.0f, 1.0f, 0.0f);
}