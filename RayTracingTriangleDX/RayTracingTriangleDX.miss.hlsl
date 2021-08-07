struct Payload
{
    float3 Color;
};

[shader("miss")]
void OnMiss(inout Payload Pay)
{
    Pay.Color = float3(0.529411793f, 0.807843208f, 0.921568692f);
}
