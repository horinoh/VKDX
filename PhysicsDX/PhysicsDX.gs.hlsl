struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
    float2 Texcoord : TEXCOORD0;
    uint InstanceID : SV_InstanceID;
};

#define INSTANCE_COUNT 100
struct TRANSFORM
{
    float4x4 Projection;
    float4x4 View;
    float4x4 World[INSTANCE_COUNT];
};
ConstantBuffer<TRANSFORM> Transform : register(b0, space0);

struct OUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
    float2 Texcoord : TEXCOORD0;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
    const float4x4 W = Transform.World[In[0].InstanceID];
    const float4x4 PVW = mul(mul(Transform.Projection, Transform.View), W);
	
    const float4x4 TexTransform = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
											0.0f, 1.0f, 0.0f, 0.0f,
											0.0f, 0.0f, 1.0f, 0.0,
											0.0f, 0.0f, 0.0f, 1.0f);
	[unroll]
	for (int i = 0; i < 3; ++i) {
        Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
        Out.Normal = mul((float3x3)W, In[i].Normal);		
        Out.Texcoord = mul(TexTransform, float4(In[i].Texcoord, 0.0f, 1.0f)).xy;
		stream.Append(Out);
	}
	stream.RestartStrip();
}
