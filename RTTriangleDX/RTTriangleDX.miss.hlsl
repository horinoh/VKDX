struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

[shader("miss")]
void OnMiss(inout PAYLOAD Payload)
{
    Payload.Color = float3(0.529411793f, 0.807843208f, 0.921568692f);
}
