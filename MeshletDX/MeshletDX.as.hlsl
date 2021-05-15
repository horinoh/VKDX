struct PAYLOAD_OUT
{
    uint MeshletIDs[32];
};

groupshared PAYLOAD_OUT Payload;

[numthreads(32, 1, 1)]
void main(uint GroupThreadID : SV_GroupThreadID, uint GroupID : SV_GroupID)
{
    bool Visible = true;

    if (Visible)
    {
        uint Index = WavePrefixCountBits(Visible);
        Payload.MeshletIDs[Index] = GroupThreadID;
    }

    uint Count = WaveActiveCountBits(Visible);
    DispatchMesh(Count, 1, 1, Payload);
}
