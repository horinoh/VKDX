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

//!< SV_GroupID             : 【C++】DispatchMesh() で発行されたどのグループか
//!< SV_GroupThreadID       : 【HLSL】 [numthreads()] で発行されたどのスレッドか (ローカル ... グループ内ID)
//!< SV_DispatchThreadID    : 【HLSL】 [numthreads()] で発行されたどのスレッドか (グローバル ... 全グループ通しID)
[numthreads(3, 1, 1)]
[outputtopology("triangle")]
void main(uint GroupThreadID : SV_GroupThreadID, uint GroupID : SV_GroupID, out indices uint3 Indices[1], out primitives PRIM_OUT Prims[1], out vertices VERT_OUT Vertices[3])
{
    /*
    V = N * N ... 4, 9, 16
    P = 2(N - 1)(N - 1) ... 2, 4, 8, 18,

    0 1 2 N
    +-+-+
    | | |
    +-+-+
    | | |
    +-+-+
    6 7 8
  */

    /*
    V = (N + 1) * (2N + 1) ... 6, 15, 28
    P = 2 * 2(N - 1)(N - 1) ... 4, 16, 36, 

    0 1 2 N
    +-+-+
    | | |   
    +-+-+      
    | | |
    +-+-+
    | | |   
    +-+-+      
    | | |
    +-+-+
    12 13 14
    */
    
    //!< スレッドグループから出力する、頂点数、プリミティブ数を宣言
    SetMeshOutputCounts(3, 1);
    
    Indices[0] = uint3(0, 1, 2);
    Prims[0].Id = 0;
 
    Vertices[GroupThreadID].Position = float4(Positions[GroupThreadID], 1.0f);
    Vertices[GroupThreadID].Color = Colors[GroupThreadID];
}
