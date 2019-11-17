struct IN
{
	uint4 Joints : JOINTS0;
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float4 Weights : WEIGHTS0;
};

struct OUT
{
	uint4 Joints : JOINTS0;
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float4 Weights : WEIGHTS0;
};

static const float Scale = 1.0f;

OUT main(IN In)
{
	OUT Out;

	Out.Joints = In.Joints;
	Out.Position = float4(In.Position * Scale, 1.0f);
	Out.Normal = In.Normal;
	Out.Weights = In.Weights;

	return Out;
}
