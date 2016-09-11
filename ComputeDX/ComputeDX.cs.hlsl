/*!
@brief �}���f���u���W��
Z(n+1) = Z(n)^2 + C
C = a + b * i;
*/
uint Mandelbrot(const float2 c, const uint iterations)
{
	float2 z = float2(0.0f, 0.0f);
	uint i = 0;
	while(i < iterations && dot(z, z) < 4.0f) {
		z = float2(z.x * z.x - z.y * z.y, 2.0f * z.x * z.y) + c;
		++i;
	}
	return i;
}

RWTexture2D<float4> TextureMap : register(u0);

static const uint3 WorkGroupSize = uint3(32, 32, 1);
[numthreads(WorkGroupSize.x, WorkGroupSize.y, WorkGroupSize.z)]
void main(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
#if 0
	//!< ���f����̈�
	//!< (x, y, z, w) = (LT������, LT������, RB������, RB������)
	static const float4 complexSpace = float4(-2.0f, -1.0f, 1.0f, 1.0f);
	uint w, h;
	TextureMap.GetDimensions(w, h);
	const float2 size = float2(w, h);

	//!< ���f����ł̃s�N�Z���T�C�Y�����߂�
	const float2 d = (complexSpace.zw - complexSpace.xy) / size;

	//!< �e�N�Z��(dispatchThreadId.xy)�𕡑f����֕ϊ�
	const float2 c = d * dispatchThreadId.xy + complexSpace.xy;

	//!< �}���f���u���W��
	const uint i = Mandelbrot(c, 100);

	const float3 color = float3(smoothstep(0, 5, i), smoothstep(5, 10, i), smoothstep(10, 15, i)) * max(sign(i - Iterations), 0);

	//!< �e�N�X�`���֏�������
	TextureMap[dispatchThreadId.xy] = float4(color, 1.0f);
#else
	//!< �i�q�͗l
	TextureMap[dispatchThreadId.xy] = float4(float2(groupThreadId.xy) / float2(WorkGroupSize.xy), 0.0f, 1.0f);
#endif

	AllMemoryBarrier();
}