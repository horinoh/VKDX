struct IN
{
	float3 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};
struct OUT
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = float4(In[i].Position, 1.0f);
		Out.Texcoord = In[i].Texcoord;
		stream.Append(Out);
	}
	stream.RestartStrip();
}
