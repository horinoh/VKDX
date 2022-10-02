struct PAYLOAD_SHADOW
{
    bool IsOccluded;
};

[shader("miss")]
void OnMiss_1(inout PAYLOAD_SHADOW Payload)
{
    Payload.IsOccluded = false;
}
