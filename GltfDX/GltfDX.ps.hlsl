struct IN
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
};

float4 main(IN In) : SV_TARGET
{
	return float4(In.Color, 1.0f);
}
