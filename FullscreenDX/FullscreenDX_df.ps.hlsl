struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

#define USE_SPHERE

float3 mod3(const float3 x, const float y)
{
	//!< GLSL : float mod(const float x, const float y) { return x - y * floor(x / y); }
	//!< HLSL : float fmod(const float x, const float y) { return x - y * trunc(x / y); }
	return x - y * floor(x / y);
	//return x - y * trunc(x / y);
}

float DistanceFunction_Sphere(const float3 Ray, const float3 Center, const float Radius)
{
	return length(Ray - Center) - Radius;
}
float3 DistanceFunction_SphereNormal(const float3 Ray, const float3 Center, const float Radius)
{
	const float3 DeltaX = float3(0.001f, 0.0f, 0.0f);
	const float3 DeltaY = float3(0.0f, 0.001f, 0.0f);
	const float3 DeltaZ = float3(0.0f, 0.0f, 0.001f);
	return normalize(float3(DistanceFunction_Sphere(Ray + DeltaX, Center, Radius) - DistanceFunction_Sphere(Ray - DeltaX, Center, Radius),
		DistanceFunction_Sphere(Ray + DeltaY, Center, Radius) - DistanceFunction_Sphere(Ray - DeltaY, Center, Radius),
		DistanceFunction_Sphere(Ray + DeltaZ, Center, Radius) - DistanceFunction_Sphere(Ray - DeltaZ, Center, Radius)));
}

float DistanceFunction_Torus(const float3 Ray, const float3 Center, const float2 Radius)
{
	const float3 Pos = Ray - Center;
	return length(float2(length(Pos.xy) - Radius.x, Pos.z)) - Radius.y;
}
float3 DistanceFunction_TorusNormal(const float3 Ray, const float3 Center, const float2 Radius)
{
	const float3 DeltaX = float3(0.001f, 0.0f, 0.0f);
	const float3 DeltaY = float3(0.0f, 0.001f, 0.0f);
	const float3 DeltaZ = float3(0.0f, 0.0f, 0.001f);
	return normalize(float3(DistanceFunction_Torus(Ray + DeltaX, Center, Radius) - DistanceFunction_Torus(Ray - DeltaX, Center, Radius),
		DistanceFunction_Torus(Ray + DeltaY, Center, Radius) - DistanceFunction_Torus(Ray - DeltaY, Center, Radius),
		DistanceFunction_Torus(Ray + DeltaZ, Center, Radius) - DistanceFunction_Torus(Ray - DeltaZ, Center, Radius)));
}

float3 Diffuse(const float3 MaterialColor, const float3 LightColor, const float LN)
{
	return saturate(saturate(LN) * MaterialColor * LightColor);
}
float3 Blinn(const float3 MaterialColor, const float4 LightColor, const float LN, const float3 L, const float3 N, const float3 V)
{
	return saturate(saturate(sign(LN)) * pow(saturate(dot(N, normalize(V + L))), LightColor.a) * LightColor.rgb * MaterialColor);
}
float3 Phong(const float3 MaterialColor, const float4 LightColor, const float LN, const float3 L, const float3 N, const float3 V)
{
	return saturate(saturate(sign(LN)) * pow(saturate(dot(reflect(-L, N), V)), LightColor.a) * LightColor.rgb * MaterialColor);
}
float3 Specular(const float3 MaterialColor, const float4 LightColor, const float LN, const float3 L, const float3 N, const float3 V)
{
	//return Blinn(MaterialColor, LightColor, LN, L, N, V);
	return Phong(MaterialColor, LightColor, LN, L, N, V);
}

float4 main(IN In) : SV_TARGET
{
	const float2 ClipSpace = In.Texcoord * 2.0f - 1.0f;

	//!< �J���� (Camera)
	const float PI = 4.0f * atan(1.0f);
	const float Speed = 0.3f;
	const float Timer = 0.0f;
	const float Radian = fmod(Timer * Speed, PI);
	const float3 CameraPos = float3(2.0f * cos(Radian), 0.0f, 2.0f * sin(Radian));
	const float3 CameraTag = float3(0.0f, 0.0f, 0.0f);
	const float3 CameraUp = float3(0.0f, 1.0f, 0.0f);

	//!< ���C (Ray)
	const float3 Forward = normalize(CameraTag - CameraPos);
	const float3 Right = normalize(cross(Forward, CameraUp));
	const float3 Up = cross(Right, Forward);
	const float Aspect = 16.0f / 9.0f;
	const float3 Ray = normalize(Forward + ClipSpace.x * Right * Aspect + ClipSpace.y * Up);

	//!< �v���~�e�B�u�̃p�����[�^ (Parameter of primitive)
	static const float3 Center = float3(0.0f, 0.0f, 0.0f);
	static const float2 Radius = float2(1.0f, 0.5f);

	//!< ���C�}�[�`���O (Ray marching)
	float3 Pos = CameraPos;
	float Distance = 0.0f;
	float Length = 0.0f;
	const float Repeate = 8.0f;
	const int Iterations = 32;
	for (int i = 0; i < Iterations; ++i) {
		//!< �����֐� : �T�[�t�F�X�ɑ΂��A�O���Ȃ琳�̒l�A�����Ȃ畉�̒l��Ԃ� (Distance function : Outside is positive, inside is negative about surface)
#ifdef USE_SPHERE
		Distance = DistanceFunction_Sphere(Pos, Center, Radius.x);
#else
		Distance = DistanceFunction_Torus(Pos, Center, Radius);
#endif

		//!< �����ɉ����āA���C��i�߂� (���������Ȃ�߂���邱�ƂɂȂ�) (Progress ray)
		Length += Distance;
		Pos = CameraPos + Ray * Length;

		//!< ���s�[�g (Repeat)
		Pos = mod3(Pos, Repeate) - Repeate * 0.5f;
	}

	//!< �V�F�[�f�B���O (Shading)
	const float3 LightDirection = float3(0.0f, -1.0f, 0.0f);
#ifdef USE_SPHERE
	const float3 N = DistanceFunction_SphereNormal(Pos, Center, Radius.x);
#else
	const float3 N = DistanceFunction_TorusNormal(Pos, Center, Radius);	
#endif
	const float3 L = normalize(LightDirection);
	const float3 V = -Ray;
	const float LN = dot(L, N);
	const float3 Ambient = float3(0.1f, 0.1f, 0.1f);
	const float3 MaterialColor = float3(0.5f, 0.5f, 0.5f);
	const float4 LightColor = float4(0.5f, 0.5f, 0.5f, 32.0f);
	const float Attenuate = 1.0;
	const float Spot = 1.0;
	const float3 Color = (Ambient + (Diffuse(MaterialColor, LightColor.rgb, LN) + Specular(MaterialColor, LightColor, LN, L, N, V)) * Attenuate) * Spot;

	//!< BG�J���[ (BG color)
	const float3 BackGroundColor = float3(0.5f, 0.5f, 1.0f);

	//!< �T�[�t�F�X��\�����邽�߁A�����̐�Βl���ق� 0.0f �Ȃ� 1 �A����ȊO�Ȃ� 0 �ɂȂ�悤�ȃ}�X�N�l
	const float Mask = max(sign(0.01f - abs(Distance)), 0.0f);
	const float InvMask = 1.0f - Mask;

	//!< �T�[�t�F�X�J���[��BG�J���[���}�X�N�l���g�p���Ĕr���I�ɕ`��
	return float4(Color * Mask + BackGroundColor * InvMask, 1.0f);
}
