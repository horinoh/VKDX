struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

[shader("intersection")]
void OnIntersection(/*in BuiltInTriangleIntersectionAttributes BITIA*/)
{
    if (false) {
        float t = 0.0f;
        uint Kind = 0;
        //ReportHit(t, Kind, Attr);
    }
}
