struct IN
{
	float3 Tangent : TANGENT;
	float3 Position : POSITION;
	float3 Normal : NORMAL;

	//!< モーフターゲット1
	float3 Tangent1 : TANGENT1;
	float3 Position1 : POSITION1;
	float3 Normal1 : NORMAL1;
	//!< モーフターゲット2
	float3 Tangent2 : TANGENT2;
	float3 Position2 : POSITION2;
	float3 Normal2 : NORMAL2;
};

struct OUT
{
	float3 Tangent : TANGENT;
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
};

static const float Scale = 30.0f;
//!< モーフウエイト
static const float W1 = 0.5f, W2 = 1.0f - W1;

OUT main(IN In)
{
	OUT Out;

	Out.Tangent = In.Tangent + W1 * In.Tangent1 + W2 * In.Tangent2;
	Out.Position = float4((In.Position + W1 * In.Position1 + W2 * In.Position2) * Scale, 1.0f);
	Out.Normal = In.Normal + W1 * In.Normal1 + W2 * In.Normal2;

	return Out;
}
