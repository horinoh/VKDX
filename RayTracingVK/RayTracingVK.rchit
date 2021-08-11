#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 Payload;
layout(location = 0) callableDataEXT vec3 CallableData;
hitAttributeEXT vec2 HitAttr;

//!< ShaderRecordBuffer �̗�
//layout(shaderRecordNV) buffer XXX {
//    uint32_t yyy;
//};

void main()
{
	Payload = vec3(1.0f - HitAttr.x - HitAttr.y, HitAttr.x, HitAttr.y);

	//!< gl_GeometryIndexEXT : BLAS �쐬���� VkAccelerationStructureBuildGeometryInfoKHR.geometryCount
	//executeCallableEXT(gl_GeometryIndexEXT, 0);

	//!< gl_InstanceCustomIndexEXT : TLAS �쐬���� VkAccelerationStructureInstanceKHR.instanceCustomIndex 
	executeCallableEXT(gl_InstanceCustomIndexEXT, 0);

	Payload = CallableData;
}