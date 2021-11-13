#define MESHLET_COUNT 32

struct PAYLOAD_OUT
{
    uint MeshletIDs[MESHLET_COUNT];
    uint MeshletChunkID;
};

groupshared PAYLOAD_OUT Payload;

//!< ���b�V�����b�g���� 32 �𒴂���ꍇ�͕�����ɕ����ĕ`�悷��
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
