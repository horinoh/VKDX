struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
};
struct OUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD0;
	float2 Depth : TEXCOORD1;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	[unroll]
	for (int i = 0; i<3; ++i) {
		//Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
		//Out.Normal = mul((float3x3)World, In[i].Normal);
		Out.Position = float4(In[i].Position, 1.0f);
		Out.Normal = In[i].Normal;
		Out.Texcoord = In[i].Texcoord;
		Out.Depth = Out.Position.zw;
		stream.Append(Out);
	}
	stream.RestartStrip();
}