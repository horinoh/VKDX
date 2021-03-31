struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD0;
};

struct TRANSFORM
{
    float4x4 Projection;
    float4x4 View;
    float4x4 World;
    float3 LocalCameraPosition;
};
ConstantBuffer<TRANSFORM> Transform : register(b0, space0);

struct OUT
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	float3 ViewDirection : TEXCOORD1;
	float3 Position3D : TEXCOORD2;
};

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
    const float3 CamPos = -float3(Transform.View[0][3], Transform.View[1][3], Transform.View[2][3]);
    const float4x4 PVW = mul(mul(Transform.Projection, Transform.View), Transform.World);
	const float4x4 TexTransform = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = mul(PVW, float4(In[i].Position, 1.0f));
		
		const float3 Binormal = cross(In[i].Normal, In[i].Tangent);

		Out.Texcoord = mul(TexTransform, float4(In[i].Texcoord, 0.0f, 1.0f)).xy;
		const float3 V = Transform.LocalCameraPosition - In[i].Position;
		Out.ViewDirection = float3(dot(V, In[i].Tangent), dot(V, Binormal), dot(V, In[i].Normal));
		
		Out.Position3D = In[i].Position;

		stream.Append(Out);
	}
	stream.RestartStrip();
}
