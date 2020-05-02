struct IN
{
	float3 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};
struct OUT
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	uint Viewport : SV_ViewportArrayIndex; 
	//uint RenderTarget : SV_RenderTargetArrayIndex; 
};

[instance(4)]
[maxvertexcount(3)]
void main(const triangle IN In[3], inout LineStream<OUT> stream, uint instanceID : SV_GSInstanceID)
{
	OUT Out;
	
	[unroll]
	for (int i = 0; i<3; ++i) {
		Out.Position = float4(In[i].Position, 1.0f);
		Out.Texcoord = In[i].Texcoord;
		Out.Viewport = instanceID; //!< GS�C���X�^���V���O(�r���[�|�[�g��)
		//Out.RenderTarget = instanceID; //!< �C���X�^���V���O(�����_�[�^�[�Q�b�g��)
		stream.Append(Out);
	}
	stream.RestartStrip();
}
