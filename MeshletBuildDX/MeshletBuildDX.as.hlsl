#define MESHLET_COUNT 32

struct PAYLOAD_OUT
{
    uint MeshletIDs[MESHLET_COUNT];
    uint MeshletChunkID;
};

groupshared PAYLOAD_OUT Payload;

//!< メッシュレット数が 32 を超える場合は複数回に分けて描画する
[numthreads(MESHLET_COUNT, 1, 1)]
void main(uint GroupThreadID : SV_GroupThreadID, uint GroupID : SV_GroupID)
{
    bool Visible = false;
   
    if (GroupThreadID < 32) {
        Visible = true;
    }
    
    if (Visible) {
        Payload.MeshletIDs[WavePrefixCountBits(Visible)] = GroupThreadID;
    }
    Payload.MeshletChunkID = GroupID;

    DispatchMesh(WaveActiveCountBits(Visible), 1, 1, Payload);
}
