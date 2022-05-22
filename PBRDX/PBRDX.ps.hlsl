struct IN
{
	float4 Position : SV_POSITION;
	float3 WorldPosition : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Binormal : BINORMAL;
	float2 Texcoord : TEXCOORD0;
	float3 CameraPosition : TEXCOORD1;
	float3 LightPosition : TEXCOORD2;
};

Texture2D NormalMap : register(t0, space0);
//!< 【PBR】
//!< 拡散反射 : 正規化ランバート + フレネル
//!< 鏡面反射 : クックトランス
Texture2D BaseColorMap : register(t1, space0); //!< ベースカラー (アルベド陰影が描かれていないこと)
Texture2D RoughnessMap : register(t2, space0);
Texture2D MetallicMap : register(t3, space0); //!< マテリアルとライトのカラーをブレンドして鏡面反射光を計算するバラメータ

SamplerState Sampler : register(s0, space0);

struct OUT
{
	float4 Color : SV_TARGET;
};

static const float PI = 4.0f * atan(1.0f);

//!< D = (1 / (4 m^2 * NH^4)) * e^((-1 / m^2) * ((1 - NH^2) / NH^2))
//!< D = (1 / (m^2 * NH^4)) * e^((NH^2 - 1) / (m^2 * NH^2))
float Beckmann(const float Microfacet, const float NH)
{
	const float NH2 = NH * NH;
	const float m2 = Microfacet * Microfacet;
	return exp(-((1.0f - NH2) / NH2) / m2) / (4.0f * m2 * NH2 * NH2);
	//return exp((NH2 - 1.0) / (NH2 * m2)) / (m2 * NH2 * NH2);
}
//!< F = 1 / 2 * ((g - LH) / (g + LH))^2 * (1 + ((LH * (g + LH) - 1) / (LH * (g - LH) + 1))^2) ... g = sqrt(n * n + LH * LH - 1)
float Fresnel(const float Metallic, const float LH)
{
	const float sq = sqrt(Metallic);
	const float n = (1.0 + sq) / (1.0 - sq);
	const float g = sqrt(n * n + LH * LH - 1.0f);
	return 0.5f * pow((g - LH) / (g + LH), 2.0f) * (1.0f + pow((LH * (g + LH) - 1.0f) / (LH * (g - LH) + 1.0f), 2.0f));
}

//!< I = (D * F * G) / (NV * NL * PI)
float CookTrrance(const float3 N, const float3 L, const float3 V, const float Metallic)
{
	//!< ライトと視線のハーフベクトル
	const float3 H = normalize(L + V);
	const float NH = saturate(dot(N, H));
	const float VH = saturate(dot(V, H));
	const float LH = saturate(dot(L, H));
	const float NL = saturate(dot(N, L));
	const float NV = saturate(dot(N, V));

	//!< D項
	const float D = Beckmann(0.76f/*小さいほどシャープな分布*/, NH);

	//!< F項
#if 1
	//!< F = Metallic + (1 - Metallic) * (1 - VH)^5 ... Schlick近似の場合
	const float F = Metallic + (1.0f - Metallic) * pow(1.0f - VH, 5.0f);
#else
	const float F = Fresnel(Metallic, LH);
#endif

	//!< G項
	//!< G = min(min(2 * NH * NV / VH, 2 * NH * NL / VH), 1)
	//const float G = min(min(2.0f * NH * NV / VH, 2.0f * NH * NL / VH), 1.0f);
	const float G = min(2.0f * NH / VH * min(NV, NL), 1.0f);

	return max(F * D * G / (NV * NL * PI), 0.0f);
}

[earlydepthstencil]
OUT main(const IN In)
{
	OUT Out;

	//!< タンジェントスペース法線
	const float3 TN = NormalMap.Sample(Sampler, In.Texcoord).xyz * 2.0f - 1.0f;
	//!< ワールドスペース法線
	const float3 WN = TN.x * normalize(In.Tangent) + TN.y * normalize(In.Binormal) + TN.z * normalize(In.Normal);
	const float3 N = WN;
	//const float3 N = normalize(In.Normal);

	//!< V : カメラへ向かうベクトル
	const float3 V = normalize(In.CameraPosition - In.WorldPosition.xyz);
	const float VN = saturate(dot(V, N)); // TODO

	//!< PBR パラメータ
	const float3 BaseColor = BaseColorMap.Sample(Sampler, In.Texcoord).rgb;
	const float3 SpecColor = BaseColor;
	const float Roughness = RoughnessMap.Sample(Sampler, In.Texcoord).r;
	const float Metallic = MetallicMap.Sample(Sampler, In.Texcoord).r;
	
	const float3 SpecM = lerp(1.0f, SpecColor, Metallic);

	//!< 環境光 (底上げ)
	const float3 Amb = 0.4f;
	float3 Color = Amb * BaseColor;

	//!< ライトの分ループ
	for (int i = 0; i < 1; ++i) {
		//!< L : 光源に向かうベクトル
		const float3 L = normalize(In.LightPosition - In.WorldPosition.xyz);
		const float3 LightColor = float3(1.0f, 1.0f, 1.0f);

		//!< 【拡散反射光】
		//!< フレネル拡散反射
		const float LN = saturate(dot(L, N));
		const float Fnl = LN * VN; 
		//!< 正規化ランバート拡散反射
		const float3 Lmb = LightColor * (LN / PI);
		//!< Diffuse
		const float3 Diffuse = BaseColor * Fnl * Lmb;

		//!< 【鏡面反射光】
		//!< Cook Trrance, Metallic
		const float3 Specular = CookTrrance(N, L, V, Metallic)* lerp(1.0f, SpecColor, Metallic); // TODO

		Color += Diffuse * Roughness + Specular;

		//Color = L * 0.5f + 0.5f;
		//Color = LN * 0.5f + 0.5f;
		//Color = VN * 0.5f + 0.5f; 
		//Color = Fnl;
		//Color = Lmb;
		Color = SpecM;
	}

	Out.Color = float4(Color, 1.0f);

	//Out.Color = float4(N * 0.5f + 0.5f, 1.0f);
	//Out.Color = float4(In.Texcoord, 0.0f, 1.0f);
	//Out.Color = float4(NormalMap.Sample(Sampler, In.Texcoord).rgb * 0.5f + 0.5f, 1.0f);
	//Out.Color = float4(BaseColorMap.Sample(Sampler, In.Texcoord).rgb, 1.0f);
	//Out.Color = float4(RoughnessMap.Sample(Sampler, In.Texcoord).rrr, 1.0f);
	//Out.Color = float4(MetallicMap.Sample(Sampler, In.Texcoord).rrr, 1.0f);

	return Out;
}

