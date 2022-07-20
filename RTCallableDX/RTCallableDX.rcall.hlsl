struct CallableDataIn
{
  float3 CallableData;
};

[shader("callable")]
void OnCallable_0(inout CallableDataIn In)
{
	//!< Žs¼–Í—l
	const float2 pos = float2(DispatchRaysIndex().xy / 8);
	In.CallableData = float3(fmod(pos.x + fmod(pos.y, 2.0f), 2.0f).xxx);
}
