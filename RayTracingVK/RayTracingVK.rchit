#version 460
#extension GL_EXT_ray_tracing : enable

#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_scalar_block_layout : enable

layout(binding = 0, set = 0) uniform accelerationStructureEXT TLAS;

layout(location = 0) rayPayloadInEXT vec3 Payload;
hitAttributeEXT vec2 HitAttr;

//uniform struct PN { vec3 Position; vec3 Normal; };
layout(buffer_reference, buffer_reference_align=8, scalar) buffer VertexBuffer { vec3 Vertices[]; };
layout(buffer_reference, buffer_reference_align=8, scalar) buffer IndexBuffer { uint Indices[]; };
layout(shaderRecordEXT) buffer SBT {
    VertexBuffer VB;
    IndexBuffer IB;
};

void main()
{
	Payload = vec3(1.0f - HitAttr.x - HitAttr.y, HitAttr.x, HitAttr.y);

    vec3 Pos[3], Nrm[3];
    for (int i = 0; i < 3; ++i) {
        Pos[i] = VB.Vertices[IB.Indices[gl_PrimitiveID * 3 + i]];
        //Nrm[i] = VB.Vertices[IB.Indices[gl_PrimitiveID * 3 + i]].Normal;
    }
    const vec3 HitPos = Pos[0] * (1.0f -  HitAttr.x - HitAttr.y) + Pos[1] * HitAttr.x + Pos[2] * HitAttr.y;
    const vec3 HitNrm = Nrm[0] * (1.0f -  HitAttr.x - HitAttr.y) + Nrm[1] * HitAttr.x + Nrm[2] * HitAttr.y;
    //const vec3 HitNrm = vec3(0,0,1);

//    Payload = vec3(0.0f);
//    const float TMin = 0.001f;
//    const float TMax = 100000.0f;
//    const vec3 Origin = vec4(HitPos, 1.0f) * gl_ObjectToWorld3x4EXT ;
//    const vec3 Direction = reflect(gl_WorldRayDirectionEXT, vec4(HitNrm, 0.0f) * gl_ObjectToWorld3x4EXT);
//    traceRayEXT(TLAS, gl_RayFlagsNoneEXT, 0xff, 0, 0, 0, Origin, TMin, Direction, TMax, 0);

    Payload = HitNrm * 0.5f + 0.5f;
}