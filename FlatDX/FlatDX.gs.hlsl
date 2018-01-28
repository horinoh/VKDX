struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
};
struct OUT
{
	float4 Position : SV_POSITION;
	/*nointerpolation*/float3 Normal : NORMAL;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = float4(In[i].Position, 1.0f);
		Out.Normal = In[i].Normal;//mul((float3x3)World, In[i].Normal);
		stream.Append(Out);
	}
	stream.RestartStrip();
}
