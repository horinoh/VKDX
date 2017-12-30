struct IN
{
	float3 Position : POSITION;
};
struct OUT
{
	float4 Position : SV_POSITION;
};

//cbuffer Transform { float4x4 Projection; float4x4 View; float4x4 World; };

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;

	//const float4x4 PVW = mul(mul(Projection, View), World);

	[unroll]
	for (int i = 0; i<3; ++i) {
		//Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
		Out.Position = float4(In[i].Position, 1.0f);
		stream.Append(Out);
	}
	stream.RestartStrip();
}
