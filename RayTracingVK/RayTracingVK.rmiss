#version 460

#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 Payload;

void main()
{
    Payload = vec3(0.529411793f, 0.807843208f, 0.921568692f);
}