#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 Payload;
hitAttributeEXT vec3 HA;

void main()
{
    //!< v0, v1, v2 �̂R���_�̏ꍇ�AhitAttributeEXT.xy �͂��ꂼ�� v1, v2 �̃E�G�C�g
    //!< v = v0 + hitAttributeEXT.x * (v1 - v0) + hitAttributeEXT.y * (v2 - v0)
    const vec3 BaryCentrics = vec3(1.0f - HA.x - HA.y, HA.x, HA.y);

	Payload = BaryCentrics;
}