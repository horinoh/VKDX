struct CallableDataIn
{
    float3 CallableData;
};

[shader("callable")]
void OnCallable_2(inout CallableDataIn In)
{
	//!< ‰¡ü
	const float2 pos = float2(DispatchRaysIndex().xy / 8);
	In.CallableData = float3(fmod(pos.y, 2.0f).xxx);
}