struct IN
{
	float4 Position : SV_POSITION;
};

float4 main(IN In) : SV_TARGET
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}
