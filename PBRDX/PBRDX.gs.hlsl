struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD0;
};

struct Transform 
{
	float4x4 Projection;
	float4x4 View; 
	float4x4 World; 
	float3 CameraPosition;
	float3 LightPosition;
};
ConstantBuffer<Transform> Tr : register(b0, space0); 

struct OUT
{
	float4 Position : SV_POSITION;
	float3 WorldPosition : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Binormal : BINORMAL;
	float2 Texcoord : TEXCOORD0;
	float3 CameraPosition : TEXCOORD1;
	float3 LightPosition : TEXCOORD2;

	float3 TangentViewDirection : TEXCOORD3;
	float3 TangentLightDirection : TEXCOORD4;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	//const float3 CamPos = -float3(Tr.View[0][3], Tr.View[1][3], Tr.View[2][3]);
	const float4x4 PVW = mul(mul(Tr.Projection, Tr.View), Tr.World);
	const float4x4 TexTransform = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	[unroll]
	for (int i = 0; i<3; ++i) {
		const float3 Binormal = cross(In[i].Normal, In[i].Tangent);
		const float4 pos4 = float4(In[i].Position, 1.0f);

		Out.Position = mul(PVW, pos4);
		Out.WorldPosition = mul(Tr.World, pos4).xyz;
		Out.Normal = mul((float3x3)Tr.World, In[i].Normal);
		Out.Tangent = mul((float3x3)Tr.World, In[i].Tangent);
		Out.Binormal = mul((float3x3)Tr.World, Binormal);
		Out.Texcoord = mul(TexTransform, float4(In[i].Texcoord, 0.0f, 1.0f)).xy;
		Out.CameraPosition = Tr.CameraPosition;
		//Out.CameraPosition = -float3(Tr.View[0][3], Tr.View[1][3], Tr.View[2][3]);
		Out.LightPosition = Tr.LightPosition;

		stream.Append(Out);
	}
	stream.RestartStrip();
}
