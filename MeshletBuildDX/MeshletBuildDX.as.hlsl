#define MESHLET_COUNT 32

struct PAYLOAD_OUT
{
    uint MeshletIDs[MESHLET_COUNT];
};

groupshared PAYLOAD_OUT Payload;

//!< メッシュレット数が 32 を超える場合は複数回に分けて描画する
[numthreads(MESHLET_COUNT, 1, 1)]
void main(uint GroupThreadID : SV_GroupThreadID, uint GroupID : SV_GroupID)
{
    bool Visible = true;
    //bool Visible = 0 == (GroupThreadID % 2);
    //bool Visible = 0 == (GroupID % 2);
    
    if (Visible)
    {
        Payload.MeshletIDs[WavePrefixCountBits(Visible)] = GroupThreadID;
    }
    DispatchMesh(WaveActiveCountBits(Visible), 1, 1, Payload);
}
