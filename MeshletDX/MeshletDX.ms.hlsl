struct PAYLOAD_IN
{
    uint MeshletIDs[32];
};

struct VERT_OUT
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};
struct PRIM_OUT
{
    uint Id : INDEX0;
};

static const float3 Positions[] = { float3(0.0f, 0.5f, 0.0f), float3(-0.5f, -0.5f, 0.0f), float3(0.5f, -0.5f, 0.0f) };
static const float3 Colors[] = { float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), float3(1.0f, 1.0f, 0.0f), float3(0.0f, 1.0f, 1.0f), float3(1.0f, 0.0f, 1.0f), float3(1.0f, 1.0f, 1.0f), float3(0.0f, 0.0f, 0.0f) };

[numthreads(3, 1, 1)]
[outputtopology("triangle")]
void main(uint GroupThreadID : SV_GroupThreadID, uint GroupID : SV_GroupID, in payload PAYLOAD_IN Payload, out indices uint3 Indices[1], out primitives PRIM_OUT Prims[1], out vertices VERT_OUT Vertices[3])
{
    //!< �X���b�h�O���[�v����o�͂���A���_���A�v���~�e�B�u����錾
    SetMeshOutputCounts(3, 1);
    
    Indices[0] = uint3(0, 1, 2);
    Prims[0].Id = 0;
 
    const float3 Offset = float3(GroupID, GroupID, 0.0f) / 32.0f - float3(0.5f, 0.5f, 0.0f);
    Vertices[GroupThreadID].Position = float4(Positions[GroupThreadID] + Offset, 1.0f);
    Vertices[GroupThreadID].Color = Colors[Payload.MeshletIDs[GroupID] % 8];
}
