#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 Payload;
layout(location = 0) callableDataEXT vec3 CallableData;
hitAttributeEXT vec2 HitAttr;

//!< ShaderRecordBuffer ‚Ì—á
//layout(shaderRecordNV) buffer XXX {
//    uint32_t yyy;
//};

void main()
{
	Payload = vec3(1.0f - HitAttr.x - HitAttr.y, HitAttr.x, HitAttr.y);

	//!< gl_GeometryIndexEXT : BLAS ì¬Žž‚Ì VkAccelerationStructureBuildGeometryInfoKHR.geometryCount
	//executeCallableEXT(gl_GeometryIndexEXT, 0);

	//!< gl_InstanceCustomIndexEXT : TLAS ì¬Žž‚Ì VkAccelerationStructureInstanceKHR.instanceCustomIndex 
	executeCallableEXT(gl_InstanceCustomIndexEXT, 0);

	Payload = CallableData;
}