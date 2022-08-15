#version 460
#extension GL_EXT_ray_tracing : enable
//!< layout(buffer_reference) に必要
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_scalar_block_layout : enable
//!< uint64_t(==VkDeviceAddress) 等の型を使えるようにする
#extension GL_EXT_shader_explicit_arithmetic_types : enable
//!< nonuniformEXT(シェーダの入力としてユニフォームでない変数に対して修飾する) に必要 
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
//!< 要エクステンション GL_EXT_buffer_reference, GL_EXT_scalar_block_layout
layout(buffer_reference, scalar/*, buffer_reference_align = 4*/) readonly buffer VertexBuffer { VertexPN Vertices[]; };
layout(buffer_reference, scalar) readonly buffer IndexBuffer { uvec3 Indices[]; };
layout(shaderRecordEXT, std430) buffer ShaderRecord
{
    VertexBuffer VB;
    IndexBuffer IB;
};
#endif

void main()
{
    if(Payload.Recursive++ >= 31) {
        Payload.Color = vec3(0.0f, 1.0f, 0.0f);
        return;
    }

    const vec3 BaryCentric = vec3(1.0f - HitAttr.x - HitAttr.y, HitAttr.x, HitAttr.y);

    //!< プリミティブインデックス : HLSL PrimitiveIndex() 相当
    const uvec3 i = IB.Indices[gl_PrimitiveID];
    VertexPN Hit;
    Hit.Position = VB.Vertices[i.x].Position * BaryCentric.x + VB.Vertices[i.y].Position * BaryCentric.y + VB.Vertices[i.z].Position * BaryCentric.z;
    Hit.Normal = normalize(VB.Vertices[i.x].Normal * BaryCentric.x + VB.Vertices[i.y].Normal * BaryCentric.y + VB.Vertices[i.z].Normal * BaryCentric.z);

    //!< gl_ObjectToWorldEXT は 4x3
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

                const float IndexAir = 1.0f; //!<【屈折率】空気
                const float IndexWater = 1.3334f; //!<【屈折率】水
                //const float IndexCrystal = 1.5443f; //!<【屈折率】水晶
                //const float IndexDiamond = 2.417f; //!<【屈折率】ダイヤモンド
                const float NL = dot(WorldNrm, gl_WorldRayDirectionEXT);
                //!< eta = 入射前 / 入射後
                if(NL < 0.0f) {
                    const float eta = IndexAir / IndexWater; //!< 屈折率の比 (In : 空気 -> 物質)
                    Direction = refract(gl_WorldRayDirectionEXT, WorldNrm, eta);
                } else {
                    const float eta = IndexWater / IndexAir; //!< 屈折率の比 (Out : 物質 -> 空気)
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