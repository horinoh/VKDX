struct IN
{
	float3 Position : POSITION;
};
struct OUT
{
	float4 Position : SV_POSITION;
    uint Viewport : SV_ViewportArrayIndex;
};

[instance(16)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout TriangleStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = float4(In[i].Position, 1.0f);
        Out.Viewport = instanceID; //!< GSインスタンシング(ビューポート毎)
		stream.Append(Out);
	}
	stream.RestartStrip();
}
