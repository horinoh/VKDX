//struct IN
//{
//    float3 Position; 
//    float4 Color;
//};
//StructuredBuffer<IN> Vertices : register(t0);
//StructuredBuffer<uint3> Indices : register(t1);

struct OUT
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

static const float3 Positions[] = { float3(0.0f, 0.5f, 0.0f), float3(-0.5f, -0.5f, 0.0f), float3(0.5f, -0.5f, 0.0f) };
static const float3 Colors[] = { float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f) };

[numthreads(32, 1, 1)]
[outputtopology("triangle")]
void main(uint ThreadID : SV_DispatchThreadID, uint GroupID : SV_GroupID, out vertices OUT Verts[3], out indices uint3 Tris[1]/*, out primitives Prims[1]*/)
{
    //!< スレッドグループから出力する、頂点数、プリミティブ数を宣言
    SetMeshOutputCounts(3, 1);
    
    Tris[0] = uint3(0, 1, 2);
    //Prims[0] = 0;
    
    Verts[0].Position = float4(Positions[0], 1.0f);
    Verts[0].Color = Colors[0];
    Verts[1].Position = float4(Positions[1], 1.0f);
    Verts[1].Color = Colors[1];
    Verts[2].Position = float4(Positions[2], 1.0f);
    Verts[2].Color = Colors[2];
       
    //!< 最初の1スレッドはインデックスを出力
    //if (GroupID < 1)
    //{
    //    Tris[GroupID] = Indices[GroupID];
    //    //Prims[GroupID] = GroupID;
    //}
    
    ////!< 最初の3スレッドは頂点を出力
    //if (GroupID < 3)
    //{
    //    OUT Out;
    //    Out.Position = float4(Vertices[GroupID].Position, 1.0f);
    //    Out.Color = Vertices[GroupID].Color;
    //    Verts[GroupID] = Out;
    //}
    }

//[numthreads(32, 1, 1)]
//[outputtopology("line")]
//void main(uint ThreadID : SV_DispatchThreadID, uint GroupID : SV_GroupID, out vertices OUTVerts[3], out indices uint2 Tris[1] /*, out primitives Prims[1]*/)
//{
//}