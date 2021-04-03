struct IN
{
    float3 Position; 
    float4 Color;
};
StructuredBuffer<IN> Vertices : register(t0);
StructuredBuffer<uint3> Indices : register(t1);

struct OUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

[numthreads(32, 1, 1)]
[outputtopology("triangle")]
void main(uint ThreadID : SV_DispatchThreadID, uint GroupID : SV_GroupID, out vertices OUT Verts[3], out indices uint3 Tris[1]/*, out primitives Prims[1]*/)
{
    //!< スレッドグループから出力する、頂点数、プリミティブ数を宣言
    SetMeshOutputCounts(3, 1);
        
    //!< 最初の1スレッドはインデックスを出力
    if (GroupID < 1)
    {
        Tris[GroupID] = Indices[GroupID];
        //Prims[GroupID] = GroupID;
    }
    
    //!< 最初の3スレッドは頂点を出力
    if (GroupID < 3)
    {
        OUT Out;
        Out.Position = float4(Vertices[GroupID].Position, 1.0f);
        Out.Color = Vertices[GroupID].Color;
        Verts[GroupID] = Out;
    }
}

//[numthreads(32, 1, 1)]
//[outputtopology("line")]
//void main(uint ThreadID : SV_DispatchThreadID, uint GroupID : SV_GroupID, out vertices OUTVerts[3], out indices uint2 Tris[1] /*, out primitives Prims[1]*/)
//{
//}