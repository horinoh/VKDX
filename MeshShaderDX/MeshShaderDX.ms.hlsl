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
    //!< �X���b�h�O���[�v����o�͂���A���_���A�v���~�e�B�u����錾
    SetMeshOutputCounts(3, 1);
        
    //!< �ŏ���1�X���b�h�̓C���f�b�N�X���o��
    if (GroupID < 1)
    {
        Tris[GroupID] = Indices[GroupID];
        //Prims[GroupID] = GroupID;
    }
    
    //!< �ŏ���3�X���b�h�͒��_���o��
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