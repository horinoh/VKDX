struct CallableDataIn
{
    float3 CallableData;
};

[shader("callable")]
void OnCallable_1(inout CallableDataIn In)
{
	//!< cü
	const float2 pos = float2(DispatchRaysIndex().xy / 8);
	In.CallableData = float3(fmod(pos.x, 2.0f).xxx);
}