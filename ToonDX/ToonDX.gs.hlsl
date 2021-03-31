struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
};

//#define USE_CBUFFER
#ifdef USE_CBUFFER
cbuffer Transform : register(b0, space0) { float4x4 Projection; float4x4 View; float4x4 World; };
#else
struct TRANSFORM
{
    float4x4 Projection;
    float4x4 View;
    float4x4 World;
};
ConstantBuffer<TRANSFORM> Transform : register(b0, space0);
#endif

struct OUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float3 ViewDirection : TEXCOORD0;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
#ifdef USE_CBUFFER
	const float3 CamPos = -float3(View[0][3], View[1][3], View[2][3]);
	const float4x4 PVW = mul(mul(Projection, View), World);
#else
    const float3 CamPos = -float3(Transform.View[0][3], Transform.View[1][3], Transform.View[2][3]);
    const float4x4 PVW = mul(mul(Transform.Projection, Transform.View), Transform.World);
#endif
	
	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
#ifdef USE_CBUFFER
		Out.Normal = mul((float3x3)World, In[i].Normal);
		Out.ViewDirection = CamPos - mul(World, Out.Position).xyz;
#else
        Out.Normal = mul((float3x3) Transform.World, In[i].Normal);
        Out.ViewDirection = CamPos - mul(Transform.World, Out.Position).xyz;
#endif
		stream.Append(Out);
    }
	stream.RestartStrip();
}
