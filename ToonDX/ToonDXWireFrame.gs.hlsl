struct IN
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
};

cbuffer Transform : register(b0, space0) { matrix Projection; matrix View; matrix World; };

struct OUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float3 ViewDirection : TEXCOORD0;
	noperspective float3 TriDistance : TEXCOORD1;
};

float3 getTriangleDistance(const float4 pos0, const float4 pos1, const float4 pos2)
{
	const matrix Viewport = matrix(1280, 0.0f, 0.0f, 1280,
		0.0f, 640, 640, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	//!< スクリーンスペース位置
	const float3 scrPos0 = mul(Viewport, (pos0 / pos0.w)).xyz;
	const float3 scrPos1 = mul(Viewport, (pos1 / pos1.w)).xyz;
	const float3 scrPos2 = mul(Viewport, (pos2 / pos2.w)).xyz;
	//!< なす角
	const float a = length(scrPos1 - scrPos2);
	const float b = length(scrPos2 - scrPos0);
	const float c = length(scrPos1 - scrPos0);
	const float alpha = acos((b * b + c * c - a * a) / (2.0f * b * c));
	const float beta = acos((a * a + c * c - b * b) / (2.0f * a * c));
	//!< 垂線
	const float ha = abs(c * sin(beta));
	const float hb = abs(c * sin(alpha));
	const float hc = abs(b * sin(alpha));
	return float3(ha, hb, hc);
}

[instance(1)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	const float3 CamPos = -float3(View[0][3], View[1][3], View[2][3]);
	const matrix PVW = mul(mul(Projection, View), World);

	const float4 pos[3] = { mul(PVW, float4(In[0].Position, 1.0f)), mul(PVW, float4(In[1].Position, 1.0f)), mul(PVW, float4(In[2].Position, 1.0f)) };
	const float3 d = getTriangleDistance(pos[0], pos[1], pos[2]);

	Out.Position = pos[0];
	//Out.Normal = mul((float3x3)World, In[0].Normal);
	Out.Normal = mul((matrix<float, 3, 3>)World, In[0].Normal);
	Out.ViewDirection = CamPos - mul(World, Out.Position).xyz;
	Out.TriDistance = float3(d.x, 0.0f, 0.0f);
	stream.Append(Out);

	Out.Position = pos[1];
	//Out.Normal = mul((float3x3)World, In[1].Normal);
	Out.Normal = mul((matrix<float, 3, 3>)World, In[1].Normal);
	Out.ViewDirection = CamPos - mul(World, Out.Position).xyz;
	Out.TriDistance = float3(0.0f, d.y, 0.0f);
	stream.Append(Out);

	Out.Position = pos[2];
	//Out.Normal = mul((float3x3)World, In[2].Normal);
	Out.Normal = mul((matrix<float, 3, 3>)World, In[2].Normal);
	Out.ViewDirection = CamPos - mul(World, Out.Position).xyz;
	Out.TriDistance = float3(0.0f, 0.0f, d.z);
	stream.Append(Out);

	stream.RestartStrip();
}
