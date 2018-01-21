struct IN
{
	float3 Position : POSITION;
};
struct OUT
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

cbuffer Transform { float4x4 Projection; float4x4 View; float4x4 World; };

[instance(1)]
[maxvertexcount(4)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	const float3 CamPos = -float3(View[0][3], View[1][3], View[2][3]);
	const float3 Axis = float3(0, 1, 0);
	const float4x4 PVW = mul(mul(Projection, View), World);

	const float3 Center = (In[0].Position + In[1].Position + In[2].Position) / 3.0f;
	const float3 Forward = normalize(CamPos - mul(World, float4(Center, 1.0f)).xyz);
	const float3 Right = cross(Axis, Forward);
	const float Scale = 0.05f;

	Out.Position = mul(PVW, float4(Center - Scale * Right - Scale * Axis, 1.0f)); //!< RB
	Out.Texcoord = float2(0.0f, 1.0f - 0.0f);
	stream.Append(Out);

	Out.Position = mul(PVW, float4(Center - Scale * Right + Scale * Axis, 1.0f)); //!< RU
	Out.Texcoord = float2(0.0f, 1.0f - 1.0f);
	stream.Append(Out);

	Out.Position = mul(PVW, float4(Center + Scale * Right - Scale * Axis, 1.0f)); //!< LB
	Out.Texcoord = float2(1.0f, 1.0f - 0.0f);
	stream.Append(Out);

	Out.Position = mul(PVW, float4(Center + Scale * Right + Scale * Axis, 1.0f)); //!< LU
	Out.Texcoord = float2(1.0f, 1.0f - 1.0f);
	stream.Append(Out);

	stream.RestartStrip();
}
