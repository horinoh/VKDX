#version 460
#extension GL_EXT_ray_tracing : enable
//!< layout(buffer_reference) �ɕK�v
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_scalar_block_layout : enable
//!< uint64_t(==VkDeviceAddress) ���̌^���g����悤�ɂ���
#extension GL_EXT_shader_explicit_arithmetic_types : enable
//!< nonuniformEXT(�V�F�[�_�̓��͂Ƃ��ă��j�t�H�[���łȂ��ϐ��ɑ΂��ďC������) �ɕK�v 
#extension GL_EXT_nonuniform_qualifier : enable

struct PAYLOAD 
{
    vec3 Color;
    int Recursive;
};
layout(location = 0) rayPayloadInEXT PAYLOAD Payload;
hitAttributeEXT vec2 HitAttr;

layout(binding = 0, set = 0) uniform accelerationStructureEXT TLAS;
struct VertexPN 
{
    vec3 Position; 
    vec3 Normal;
};
#if 1
layout(binding = 4, set = 0) readonly buffer VertexBuffer { VertexPN Vertices[]; } VB;
layout(binding = 5, set = 0) readonly buffer IndexBuffer { uvec3 Indices[]; } IB;
#else
//!< �v�G�N�X�e���V���� GL_EXT_buffer_reference, GL_EXT_scalar_block_layout
layout(buffer_reference, scalar/*, buffer_reference_align = 4*/) readonly buffer VertexBuffer { VertexPN Vertices[]; };
layout(buffer_reference, scalar) readonly buffer IndexBuffer { uvec3 Indices[]; };
layout(shaderRecordEXT, std430) buffer ShaderRecord
{
    VertexBuffer VB;
    IndexBuffer IB;
};
#endif

vec3 ToBaryCentric(const vec2 Attr)
{
    return vec3(1.0f - Attr.x - Attr.y, Attr.x, Attr.y);
}
vec3 ToCartesian(const vec3 A, const vec3 B, const vec3 C, const vec3 BaryCentric)
{
    return BaryCentric.x * A + BaryCentric.y * B + BaryCentric.z * C;
}
vec3 ToCartesian(const vec3 V[3], const vec3 BaryCentric)
{
    return ToCartesian(V[0], V[1], V[2], BaryCentric);
}

void main()
{
    if(Payload.Recursive++ >= 31) {
        Payload.Color = vec3(0.0f, 1.0f, 0.0f);
        return;
    }

    const vec3 BaryCentric = ToBaryCentric(HitAttr);

    //!< �v���~�e�B�u�C���f�b�N�X : HLSL PrimitiveIndex() ����
    const uvec3 Index = IB.Indices[gl_PrimitiveID];
    VertexPN Hit;
    Hit.Position = ToCartesian(VB.Vertices[Index.x].Position, VB.Vertices[Index.y].Position, VB.Vertices[Index.z].Position, BaryCentric);
    Hit.Normal = ToCartesian(VB.Vertices[Index.x].Normal, VB.Vertices[Index.y].Normal, VB.Vertices[Index.z].Normal, BaryCentric);

    //!< gl_ObjectToWorldEXT �� 4x3
    const vec3 WorldPos = gl_ObjectToWorldEXT * vec4(Hit.Position, 1.0f);
    const vec3 WorldNrm = normalize(mat3(gl_ObjectToWorld3x4EXT) * Hit.Normal);

    switch(gl_InstanceCustomIndexEXT) {
        case 0:
            {
                const float TMin = 0.001f;
                const float TMax = 100000.0f;
                const vec3 Origin = WorldPos;
                const vec3 Direction = reflect(gl_WorldRayDirectionEXT, WorldNrm);
                traceRayEXT(TLAS, gl_RayFlagsNoneEXT, 0xff, 0, 0, 0, Origin, TMin, Direction, TMax, 0);
            }
            break;
        case 1:
            {
                const float TMin = 0.001f;
                const float TMax = 100000.0f;
                const vec3 Origin = WorldPos;
                vec3 Direction = vec3(0.0f);

                const float IndexAir = 1.0f; //!<�y���ܗ��z��C
                const float IndexWater = 1.3334f; //!<�y���ܗ��z��
                //const float IndexCrystal = 1.5443f; //!<�y���ܗ��z����
                //const float IndexDiamond = 2.417f; //!<�y���ܗ��z�_�C�������h
                const float NL = dot(WorldNrm, gl_WorldRayDirectionEXT);
                //!< eta = ���ˑO / ���ˌ�
                if(NL < 0.0f) {
                    const float eta = IndexAir / IndexWater; //!< ���ܗ��̔� (In : ��C -> ����)
                    Direction = refract(gl_WorldRayDirectionEXT, WorldNrm, eta);
                } else {
                    const float eta = IndexWater / IndexAir; //!< ���ܗ��̔� (Out : ���� -> ��C)
                    Direction = refract(gl_WorldRayDirectionEXT, WorldNrm, eta);
                }

                if(length(Direction) < 0.001f) {
                    //!< Reflect
                    Direction = reflect(gl_WorldRayDirectionEXT, WorldNrm);
                    traceRayEXT(TLAS, gl_RayFlagsNoneEXT, 0xff, 0, 0, 0, Origin, TMin, Direction, TMax, 0);
                } else {
                    traceRayEXT(TLAS, gl_RayFlagsNoneEXT, 0xff, 0, 0, 0, Origin, TMin, Direction, TMax, 0);
                }
            }
            break;
        case 2:
            Payload.Color = Hit.Normal * 0.5f + 0.5f;
            break;
    }
}