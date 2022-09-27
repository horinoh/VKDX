struct CALLABLEDATA
{
    float3 Data;
};

[shader("callable")]
void OnCallable_2(inout CALLABLEDATA In)
{
	//!< ‰¡ü
	const float2 pos = float2(DispatchRaysIndex().xy / 8);
	In.Data = float3(fmod(pos.y, 2.0f).xxx);
}