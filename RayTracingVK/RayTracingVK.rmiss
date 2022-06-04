#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 Payload;

layout (binding = 4, set = 0) uniform samplerCube CubeMap;

void main()
{
    //!< 自動的にミップマップを決定できないので textureLod() で明示的に指定する
    Payload = textureLod(CubeMap, gl_WorldRayDirectionEXT, 0.0f).rgb;
}