struct PAYLOAD_OUT
{
    uint MeshletIDs[32];
};

groupshared PAYLOAD_OUT Payload;

[numthreads(1, 1, 1)]
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
