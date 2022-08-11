#version 460
#extension GL_EXT_ray_tracing : enable

struct PAYLOAD 
{
    vec3 Color;
    int Recursive;
};
layout(location = 0) rayPayloadInEXT PAYLOAD Payload;

layout (binding = 2, set = 0) uniform samplerCube CubeMap;

void main()
{
    //!< 自動的にミップマップを決定できないので textureLod() で明示的にミップレベルを指定する
    Payload.Color = textureLod(CubeMap, gl_WorldRayDirectionEXT, 0.0f).rgb;
}