struct IN
{
	float3 Position : POSITION;
};

struct TRANSFORM
{
    float4x4 Projection;
    float4x4 View;
    float4x4 World;
    float4x4 LightProjection;
    float4x4 LightView;
};
ConstantBuffer<TRANSFORM> Transform : register(b0, space0);

struct OUT
{
	float4 Position : SV_POSITION;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
    const float4x4 PVW = mul(mul(Transform.Projection, Transform.View), Transform.World);

	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
		stream.Append(Out);
	}
	stream.RestartStrip();
}
