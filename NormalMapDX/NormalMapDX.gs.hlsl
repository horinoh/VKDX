struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD0;
};

//cbuffer Transform { float4x4 Projection; float4x4 View; float4x4 World; };

struct OUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float4 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD0;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	//const float4x4 PVW = mul(mul(Projection, View), World);

	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = float4(In[i].Position, 1.0f);//mul(PVW, float4(In[i].Position, 1.0f));
		Out.Normal = In[i].Normal;//mul((float3x3)World, In[i].Normal);
		Out.Tangent = float4(In[i].Tangent, 1.0f);//float4(mul((float3x3)World, In[i].Tangent), 1.0f);
		Out.Texcoord = In[i].Texcoord;//mul(TextureTransform, float4(In[i].Texcoord, 0.0f, 1.0f)).xy;
		stream.Append(Out);
	}
	stream.RestartStrip();
}
