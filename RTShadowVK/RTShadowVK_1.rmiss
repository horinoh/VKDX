#version 460
#extension GL_EXT_ray_tracing : enable

struct PAYLOAD_SHADOW
{
    bool IsOccluded;
};
layout(location = 1) rayPayloadInEXT PAYLOAD_SHADOW PayloadShadow;

void main()
{
    PayloadShadow.IsOccluded = false;
}