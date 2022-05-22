#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InWorldPosition;
layout (location = 1) in vec3 InNormal;
layout (location = 2) in vec3 InTangent;
layout (location = 3) in vec3 InBinormal;
layout (location = 4) in vec2 InTexcoord;
layout (location = 5) in vec3 InCameraPosition;
layout (location = 6) in vec3 InLightPosition;

layout (set = 0, binding = 1) uniform sampler2D NormalMap;
//!< �yPBR�z
//!< �g�U���� : ���K�������o�[�g + �t���l��
//!< ���ʔ��� : �N�b�N�g�����X
layout (set = 0, binding = 2) uniform sampler2D BaseColorMap; //!< �x�[�X�J���[ (�A���x�h : �A�e���`����Ă��Ȃ�����)
layout (set = 0, binding = 3) uniform sampler2D RoughnessMap;
layout (set = 0, binding = 4) uniform sampler2D MetallicMap; //!< �}�e���A���ƃ��C�g�̃J���[���u�����h���ċ��ʔ��ˌ����v�Z����o�����[�^

layout (location = 0) out vec4 OutColor;

const float PI = 4.0f * atan(1.0f);

//!< D = (1 / (4 m^2 * NH^4)) * e^((-1 / m^2) * ((1 - NH^2) / NH^2))
//!< D = (1 / (m^2 * NH^4)) * e^((NH^2 - 1) / (m^2 * NH^2))
float Beckmann(const float Microfacet, const float NH)
{
    const float NH2 = NH * NH;
    const float m2 = Microfacet * Microfacet;
    return exp(-((1.0f - NH2)/ NH2) / m2) / (4.0f * m2 * NH2 * NH2);
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
float CookTrrance(const vec3 N, const vec3 L, const vec3 V, const float Metallic)
{
	//!< ���C�g�Ǝ����̃n�[�t�x�N�g��
	const vec3 H = normalize(L + V);
	const float NH = clamp(dot(N, H), 0.0f, 1.0f);
    const float VH = clamp(dot(V, H), 0.0f, 1.0f);
    const float LH = clamp(dot(L, H), 0.0f, 1.0f);
    const float NL = clamp(dot(N, L), 0.0f, 1.0f);
    const float NV = clamp(dot(N, V), 0.0f, 1.0f);

	//!< D��(�}�C�N���t�@�Z�b�g���z)
	const float D = Beckmann(0.76f/*�������قǃV���[�v�ȕ��z*/, NH);

	//!< F��(�t���l��)
#if 1
	//!< F = Metallic + (1 - Metallic) * (1 - VH)^5 ... Schlick�ߎ��̏ꍇ
	const float F = Metallic + (1.0f - Metallic) * pow(1.0f - VH, 5.0f);
#else
	const float F = Fresnel(Metallic, LH);
#endif

	//!< G��(�􉽌�����)
	//!< G = min(min(2 * NH * NV / VH, 2 * NH * NL / VH), 1)
	//const float G = min(min(2.0f * NH * NV / VH, 2.0f * NH * NL / VH), 1.0f);
	const float G = min(2.0f * NH / VH * min(NV, NL), 1.0f);

	return max(F * D * G / (NV * NL * PI), 0.0f);
}

layout (early_fragment_tests) in;
void main()
{
	//!< �^���W�F���g�X�y�[�X�@��
	const vec3 TN = texture(NormalMap, InTexcoord).xyz * 2.0f - 1.0f;
	//!< ���[���h�X�y�[�X�@��
	const vec3 WN = TN.x * normalize(InTangent) + TN.y * normalize(InBinormal) + TN.z * normalize(InNormal);
	const vec3 N = WN;
	//const vec3 N = normalize(InNormal);

	//!< V : �J�����֌������x�N�g��
	const vec3 V = normalize(InCameraPosition - InWorldPosition);	
	const float VN = clamp(dot(V, N), 0.0f, 1.0f); // TODO

	//!< PBR �p�����[�^
	const vec3 BaseColor = texture(BaseColorMap, InTexcoord).rgb;
	const vec3 SpecColor = BaseColor;
	const float Roughness = texture(RoughnessMap, InTexcoord).r;
	const float Metallic = texture(MetallicMap, InTexcoord).r;

	const vec3 SpecM = mix(vec3(1.0f), SpecColor, Metallic);

	//!< ���� (��グ)
	const vec3 Amb = vec3(0.4f);
	vec3 Color = Amb * BaseColor;

	//!< ���C�g�̕����[�v
	for(int i = 0; i < 1;++i){
		//!< L : �����Ɍ������x�N�g��
		const vec3 L = normalize(InLightPosition - InWorldPosition);
		const vec3 LightColor = vec3(1.0f, 1.0f, 1.0f);
		
		//!< �y�g�U���ˌ��z
		//!< �t���l���g�U����
		const float LN = clamp(dot(L, N), 0.0f, 1.0f);
		const float Fnl = LN * VN;
		//!< ���K�������o�[�g�g�U����
		const vec3 Lmb = LightColor * (LN / PI);
		//!< Diffuse
		const vec3 Diffuse = BaseColor * Fnl * Lmb;

		//!< �y���ʔ��ˌ��z
		//!< Cook Trrance, Metallic
		const vec3 Specular = CookTrrance(N, L, V, Metallic) * mix(vec3(1.0f), SpecColor, Metallic); // TODO

		Color += Diffuse * Roughness + Specular;

		//Color = L * 0.5f + 0.5f;
		//Color = vec3(LN) * 0.5f + 0.5f;
		///Color = vec3(VN) * 0.5f + 0.5f;
		//Color = vec3(Fnl); 
		//Color = Lmb;
		Color = SpecM;
	}

	OutColor = vec4(Color, 1.0f);

	//OutColor = vec4(N * 0.5f + 0.5f, 1.0f);
	//OutColor = vec4(InTexcoord, 0.0f, 1.0f);
	//OutColor = vec4(texture(NormalMap, InTexcoord).rgb * 0.5f + 0.5f, 1.0f);
	//OutColor = vec4(texture(BaseColorMap, InTexcoord).rgb, 1.0f);
	//OutColor = vec4(texture(RoughnessMap, InTexcoord).rrr, 1.0f);
	//OutColor = vec4(texture(MetallicMap, InTexcoord).rrr, 1.0f);
}