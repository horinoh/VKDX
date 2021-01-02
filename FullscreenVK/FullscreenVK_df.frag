#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

#define USE_SPHERE

float DistanceFunction_Sphere(const vec3 Ray, const vec3 Center, const float Radius)
{
	return length(Ray - Center) - Radius;
}
vec3 DistanceFunction_SphereNormal(const vec3 Ray, const vec3 Center, const float Radius)
{
#if 1
	const vec3 DeltaX = vec3(0.001f, 0.0f, 0.0f);
	const vec3 DeltaY = vec3(0.0f, 0.001f, 0.0f);
	const vec3 DeltaZ = vec3(0.0f, 0.0f, 0.001f);
	return normalize(vec3(DistanceFunction_Sphere(Ray + DeltaX, Center, Radius) - DistanceFunction_Sphere(Ray - DeltaX, Center, Radius),
		                    DistanceFunction_Sphere(Ray + DeltaY, Center, Radius) - DistanceFunction_Sphere(Ray - DeltaY, Center, Radius),
		                    DistanceFunction_Sphere(Ray + DeltaZ, Center, Radius) - DistanceFunction_Sphere(Ray - DeltaZ, Center, Radius)));
#else
	//!< �X�N���[����Ԃ̍���xy�̊O��
	return normalize(cross(dFdx(Ray), dFdy(Ray)));
#endif
}

float DistanceFunction_Torus(const vec3 Ray, const vec3 Center, const vec2 Radius)
{
	const vec3 Pos = Ray - Center;
	return length(vec2(length(Pos.xy) - Radius.x, Pos.z)) - Radius.y;
}
vec3 DistanceFunction_TorusNormal(const vec3 Ray, const vec3 Center, const vec2 Radius)
{
#if 1
	const vec3 DeltaX = vec3(0.001f, 0.0f, 0.0f);
	const vec3 DeltaY = vec3(0.0f, 0.001f, 0.0f);
	const vec3 DeltaZ = vec3(0.0f, 0.0f, 0.001f);
	return normalize(vec3(DistanceFunction_Torus(Ray + DeltaX, Center, Radius) - DistanceFunction_Torus(Ray - DeltaX, Center, Radius),
		                    DistanceFunction_Torus(Ray + DeltaY, Center, Radius) - DistanceFunction_Torus(Ray - DeltaY, Center, Radius),
		                    DistanceFunction_Torus(Ray + DeltaZ, Center, Radius) - DistanceFunction_Torus(Ray - DeltaZ, Center, Radius)));
#else
	return normalize(cross(dFdx(Ray), dFdy(Ray)));
#endif
}

vec3 Diffuse(const vec3 MaterialColor, const vec3 LightColor, const float LN)
{
	return clamp(clamp(LN, 0.0f, 1.0f) * MaterialColor * LightColor, 0.0f, 1.0f);
}
vec3 Blinn(const vec3 MaterialColor, const vec4 LightColor, const float LN, const vec3 L, const vec3 N, const vec3 V)
{
	return clamp(clamp(sign(LN), 0.0f, 1.0f) * pow(clamp(dot(N, normalize(V + L)), 0.0f, 1.0f), LightColor.a) * LightColor.rgb * MaterialColor, 0.0f, 1.0f);
}
vec3 Phong(const vec3 MaterialColor, const vec4 LightColor, const float LN, const vec3 L, const vec3 N, const vec3 V)
{
	return clamp(clamp(sign(LN), 0.0f, 1.0f) * pow(clamp(dot(reflect(-L, N), V), 0.0f, 1.0f), LightColor.a) * LightColor.rgb * MaterialColor, 0.0f, 1.0f);
}
vec3 Specular(const vec3 MaterialColor, const vec4 LightColor, const float LN, const vec3 L, const vec3 N, const vec3 V)
{
	//return Blinn(MaterialColor, LightColor, LN, L, N, V);
	return Phong(MaterialColor, LightColor, LN, L, N, V);
}

void main()
{
	const vec2 ClipSpace = InTexcoord * 2.0f - 1.0f;

	//!< �J���� (Camera)
	const float PI = 4.0f * atan(1.0f);
	const float Speed = 0.3f;
	const float Timer = 0.0f;
	const float Radian = mod(Timer * Speed, PI);
	const vec3 CameraPos = vec3(2.0f * cos(Radian), 0.0f, 2.0f * sin(Radian));
	const vec3 CameraTag = vec3(0.0f, 0.0f, 0.0f);
	const vec3 CameraUp = vec3(0.0f, 1.0f, 0.0f);

	//!< ���C (Ray)
	const vec3 Forward = normalize(CameraTag - CameraPos);
	const vec3 Right = normalize(cross(Forward, CameraUp));
	const vec3 Up = cross(Right, Forward);
	const float Aspect = 16.0 / 9.0f;
	const vec3 Ray = normalize(Forward + ClipSpace.x * Right * Aspect + ClipSpace.y * Up);

	//!< �v���~�e�B�u�̃p�����[�^ (Parameter of primitive)
	const vec3 Center = vec3(0.0f, 0.0f, 0.0f);
	const vec2 Radius = vec2(1.0f, 0.5f);

	//!< ���C�}�[�`���O (Ray marching)
	vec3 Pos = CameraPos;
	float Distance = 0.0f;
	float Length = 0.0f;
	const float Repeate = 8.0f;
	const int Iterations = 32;;
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
		Pos = mod(Pos, Repeate) - Repeate * 0.5f;
	}

	//!< �V�F�[�f�B���O (Shading)
	const vec3 LightDirection = vec3(0.0f, -1.0f, 0.0f);
#ifdef USE_SPHERE
	const vec3 N = DistanceFunction_SphereNormal(Pos, Center, Radius.x);
#else
	const vec3 N = DistanceFunction_TorusNormal(Pos, Center, Radius);	
#endif
	const vec3 L = normalize(LightDirection);
	const vec3 V = -Ray;
	const float LN = dot(L, N);
	const vec3 Ambient = vec3(0.1f, 0.1f, 0.1f);
	const vec3 MaterialColor = vec3(0.5f, 0.5f, 0.5f);
	const vec4 LightColor = vec4(0.5f, 0.5f, 0.5f, 32.0f);
	const float Attenuate = 1.0;
	const float Spot = 1.0;
	const vec3 Color = (Ambient + (Diffuse(MaterialColor, LightColor.rgb, LN) + Specular(MaterialColor, LightColor, LN, L, N, V)) * Attenuate) * Spot;

	//!< BG�J���[ (BG color)
	const vec3 BackGroundColor = vec3(0.529411793f, 0.807843208f, 0.921568692f);

	//!< �T�[�t�F�X��\�����邽�߁A�����̐�Βl���ق� 0.0f �Ȃ� 1 �A����ȊO�Ȃ� 0 �ɂȂ�悤�ȃ}�X�N�l (Mask value which is 1 when distance nearly equal 0, otherwise 0)
	const float Mask = max(sign(0.01f - abs(Distance)), 0.0f);
	const float InvMask = 1.0f - Mask;

	//!< �T�[�t�F�X�J���[��BG�J���[���}�X�N�l���g�p���Ĕr���I�ɕ`�� (Exclusively draw surface and BG color, using mask value)
	OutColor = vec4(Color * Mask + BackGroundColor * InvMask, 1.0f);
}