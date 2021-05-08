struct OUT
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

static const float3 Positions[] = { float3(0.0f, 0.5f, 0.0f), float3(-0.5f, -0.5f, 0.0f), float3(0.5f, -0.5f, 0.0f) };
static const float3 Colors[] = { float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f) };

[numthreads(3, 1, 1)]
[outputtopology("triangle")]
//[outputtopology("line")]
void main(uint ThreadID : SV_DispatchThreadID, uint GroupID : SV_GroupID, out vertices OUT Vertices[3], out indices uint3 Indices[1]/*, out primitives Prims[1]*/)
{
    //!< スレッドグループから出力する、頂点数、プリミティブ数を宣言
    SetMeshOutputCounts(3, 1);
    
    Indices[0] = uint3(0, 1, 2);
    //Prims[0] = 0;
 
    Vertices[ThreadID].Position = float4(Positions[ThreadID], 1.0f);
    Vertices[ThreadID].Color = Colors[ThreadID];
    //Vertices[0].Position = float4(Positions[0], 1.0f);
    //Vertices[0].Color = Colors[0];
    //Vertices[1].Position = float4(Positions[1], 1.0f);
    //Vertices[1].Color = Colors[1];
    //Vertices[2].Position = float4(Positions[2], 1.0f);
    //Vertices[2].Color = Colors[2];       
}
