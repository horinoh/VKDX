struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};
struct OUT
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	uint Viewport : SV_ViewportArrayIndex; 
};

[instance(4)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = In[i].Position;
		Out.Texcoord = In[i].Texcoord;
		//!< 分割画面ではビューポート1以降を使用する
		Out.Viewport = instanceID + 1;
		stream.Append(Out);
	}
	stream.RestartStrip();
}
