struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
    uint InstanceID : SV_InstanceID;
};

#define INSTANCE_COUNT 10
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
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
    const float4x4 W = Transform.World[In[0].InstanceID];
    const float4x4 PVW = mul(mul(Transform.Projection, Transform.View), W);
	
	[unroll]
	for (int i = 0; i < 3; ++i) {
        Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
        Out.Normal = mul((float3x3)W, In[i].Normal);		
		stream.Append(Out);
	}
	stream.RestartStrip();
}
