struct CALLABLEDATA
{
  float3 Data;
};

[shader("callable")]
void OnCallable_0(inout CALLABLEDATA In)
{
	//!< Žs¼–Í—l
	const float2 pos = float2(DispatchRaysIndex().xy / 8);
	In.Data = float3(fmod(pos.x + fmod(pos.y, 2.0f), 2.0f).xxx);
}
