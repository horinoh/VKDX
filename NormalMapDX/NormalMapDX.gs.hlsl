struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD0;
};

struct Transform { float4x4 Projection; float4x4 View; float4x4 World; float3 LocalCameraPosition; float3 LocalLightDirection; };
ConstantBuffer<Transform> Tr : register(b0, space0); 

struct OUT
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	float3 ViewDirection : TEXCOORD1;
	float3 LightDirection : TEXCOORD2;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	const float3 CamPos = -float3(Tr.View[0][3], Tr.View[1][3], Tr.View[2][3]);
	const float4x4 PVW = mul(mul(Tr.Projection, Tr.View), Tr.World);
	const float4x4 TexTransform = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
		
		const float3 Binormal = cross(In[i].Normal, In[i].Tangent);

		Out.Texcoord = mul(TexTransform, float4(In[i].Texcoord, 0.0f, 1.0f)).xy;
		const float3 V = Tr.LocalCameraPosition - In[i].Position;
		Out.ViewDirection = float3(dot(V, In[i].Tangent), dot(V, Binormal), dot(V, In[i].Normal));
		Out.LightDirection = float3(dot(Tr.LocalLightDirection, In[i].Tangent), dot(Tr.LocalLightDirection, Binormal), dot(Tr.LocalLightDirection, In[i].Normal));
		
		stream.Append(Out);
	}
	stream.RestartStrip();
}
