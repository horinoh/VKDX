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
static const float3 Colors[] = { float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f) };

[numthreads(3, 1, 1)]
[outputtopology("triangle")]
void main(uint ThreadID : SV_GroupThreadID, uint GroupID : SV_GroupID, out indices uint3 Indices[1], out primitives PRIM_OUT Prims[1], out vertices VERT_OUT Vertices[3])
{
    //!< スレッドグループから出力する、頂点数、プリミティブ数を宣言
    SetMeshOutputCounts(3, 1);
    
    Indices[0] = uint3(0, 1, 2);
    Prims[0].Id = 0;
 
    Vertices[ThreadID].Position = float4(Positions[ThreadID], 1.0f);
    Vertices[ThreadID].Color = Colors[ThreadID];
}
