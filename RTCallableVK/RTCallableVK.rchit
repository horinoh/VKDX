#version 460
#extension GL_EXT_ray_tracing : enable

struct PAYLOAD 
{
    vec3 Color;
    int Recursive;
};
layout(location = 0) rayPayloadInEXT PAYLOAD Payload;
struct CALLABLEDATA
{
    vec3 Data;
};
layout(location = 0) callableDataEXT CALLABLEDATA CallableData;
hitAttributeEXT vec2 HitAttr;

void main()
{
    if(Payload.Recursive++ >= 1) {
        Payload.Color = vec3(0.0f, 1.0f, 0.0f);
        return;
    }

	//!< [C++] VkAccelerationStructureInstanceKHR.instanceCustomIndex, [GLSL] gl_InstanceCustomIndexEXT
	executeCallableEXT(gl_InstanceCustomIndexEXT, 0);

	//!< ��
	Payload.Color = CallableData.Data * vec3(1.0f, 0.0f, 0.0f);
}