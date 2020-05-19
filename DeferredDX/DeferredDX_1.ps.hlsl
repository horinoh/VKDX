struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

SamplerState Sampler : register(s0, space0);
Texture2D Texture : register(t0, space0);	//!< �J���[(Color)
Texture2D Texture1 : register(t1, space0);	//!< �@��(Normal)
Texture2D Texture2 : register(t2, space0);	//!< �[�x(Depth)
Texture2D Texture3 : register(t3, space0);	//!< ����

float4 main(IN In) : SV_TARGET
{
	return Texture.Sample(Sampler, In.Texcoord);
	//!< UV�Ɛ[�x���烏�[���h�ʒu�����߂�
	//const float4 Tmp = mul(InvViewProjection, float4(InTexcoord * 2.0f - 1.0f, Texture/*2*/.Sample(Sampler, In.Texcoord).r, 1.0f));
	//const float3 WorldPos = Tmp.xyz / Tmp.w;
}
