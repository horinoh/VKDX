#version 460
#extension GL_EXT_ray_tracing : enable

layout (binding = 1, set = 0) uniform samplerCube CubeMap;

layout(location = 0) rayPayloadInEXT vec3 Payload;

void main()
{
    Payload = textureLod(CubeMap, gl_WorldRayDirectionEXT, 0.0f).rgb;
    Payload = vec3(0.529411793f, 0.807843208f, 0.921568692f);
}